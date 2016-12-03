from __future__ import division
from __future__ import print_function

import sys
import os

import Queue
import threading
import random
import time

import numpy as np
from numpy.random import choice
from skimage.io import imread
from skimage.color import gray2rgb, rgba2rgb
from skimage.transform import resize
from skimage.transform import SimilarityTransform
from skimage.transform import AffineTransform
from skimage.transform import warp
from skimage.filters import rank
from skimage.morphology import disk
from skimage.exposure import equalize_adapthist
from skimage.exposure import equalize_hist
from skimage.exposure import adjust_gamma

from joblib import Parallel, delayed
# from multiprocessing import Pool


class BaseBatchIterator(object):
    def __init__(self, batch_size, shuffle=False, verbose=False):
        self.batch_size = batch_size
        self.verbose = False
        self.shuffle = shuffle

    def __call__(self, X, y=None):
        self.X, self.y = X, y
        return self

    def __iter__(self):
        n_samples = self.X.shape[0]
        bs = self.batch_size
        n_batches = (n_samples + bs - 1) // bs

        if self.shuffle:
            idx = np.random.permutation(len(self.X))
        else:
            idx = range(len(self.X))

        for i in range(n_batches):
            sl = slice(i * bs, (i + 1) * bs)
            Xb = self.X[idx[sl]]
            if self.y is not None:
                yb = self.y[idx[sl]]
            else:
                yb = None
            yield self.transform(Xb, yb)

    @property
    def n_samples(self):
        X = self.X
        if isinstance(X, dict):
            return len(list(X.values())[0])
        else:
            return len(X)

    def transform(self, Xb, yb):
        return Xb, yb

    def __getstate__(self):
        state = dict(self.__dict__)
        for attr in ('X', 'y',):
            if attr in state:
                del state[attr]
        return state


class ShuffleBatchIteratorMixin(object):
    """
    From https://github.com/dnouri/nolearn/issues/27#issuecomment-71175381

    Shuffle the order of samples
    """
    def __iter__(self):
        orig_X, orig_y = self.X, self.y
        self.X, self.y = shuffle(self.X, self.y)

        for res in super(ShuffleBatchIteratorMixin, self).__iter__():
            yield res

        self.X, self.y = orig_X, orig_y


class RebalanceBatchIteratorMixin(object):
    """
    Rebalance samples at each iteration according to the given per-label weights
    """
    def __init__(self, rebalance_weights, *args, **kwargs):
        super(RebalanceBatchIteratorMixin, self).__init__(*args, **kwargs)
        self.rebalance_weights = rebalance_weights
        self._printed = False

    def __iter__(self):
        X, y = self.X, self.y
        X_orig = X
        y_orig = y
        assert y.ndim == 1

        n = len(X)
        ydist = np.bincount(y).astype(float) / len(y)
        idx = np.arange(n)

        # Create sampling probablity list based on the target
        # per-label weights
        p = np.zeros_like(idx, dtype=float)
        for dist, (label, target_dist) in zip(ydist, enumerate(self.rebalance_weights)):
            p[y == label] = target_dist / dist
        p /= p.sum()

        idx = np.random.choice(idx, size=n, p=p)

        X = X[idx]
        y = y[idx]

        assert len(X) == len(X_orig)
        assert len(y) == len(y_orig)

        for res in super(RebalanceBatchIteratorMixin, self).__iter__():
            yield res

        self.X, self.y = X_orig, y_orig


class BufferedBatchIteratorMixin(object):
    """
    Create a buffered iterator which the next batch will be generated
    from a new thread. Help to speed up training if there is significant
    image preprocessing.

    Should be the last mixin
    """
    def __init__(self, buffer_size=2, *args, **kwargs):
        super(BufferedBatchIteratorMixin, self).__init__(*args, **kwargs)
        self.buffer_size = buffer_size

    def __iter__(self):
        gen = super(BufferedBatchIteratorMixin, self).__iter__()
        return make_buffer_for_iterator(gen, self.buffer_size)


class BufferedThreadedBatchIteratorMixin(object):
    def __init__(self, buffer_size=2, n_jobs=2, *args, **kwargs):
        super(BufferedThreadedBatchIteratorMixin, self).__init__(*args, **kwargs)
        self.buffer_size = buffer_size
        self.n_workers = 2

    def __iter__(self):
        gen = super(BufferedThreadedBatchIteratorMixin, self).__iter__()
        return make_buffer_for_iterator_with_thread(gen, self.n_workers, self.buffer_size)


class AffineTransformBatchIteratorMixin(object):
    """
    Apply affine transform (scale, translate and rotation)
    with a random chance
    """
    def __init__(self, affine_p,
                 affine_scale_choices=[1.], affine_translation_choices=[0.],
                 affine_rotation_choices=[0.], affine_shear_choices=[0.],
                 affine_transform_bbox=False,
                 *args, **kwargs):
        super(AffineTransformBatchIteratorMixin,
              self).__init__(*args, **kwargs)
        self.affine_p = affine_p
        self.affine_scale_choices = affine_scale_choices
        self.affine_translation_choices = affine_translation_choices
        self.affine_rotation_choices = affine_rotation_choices
        self.affine_shear_choices = affine_shear_choices

        if self.verbose:
            print('Random transform probability: %.2f' % self.affine_p)
            print('Rotation choices', self.affine_rotation_choices)
            print('Scale choices', self.affine_scale_choices)
            print('Translation choices', self.affine_translation_choices)
            print('Shear choices', self.affine_shear_choices)

    def transform(self, Xb, yb):
        Xb, yb = super(AffineTransformBatchIteratorMixin,
                       self).transform(Xb, yb)
        # Skip if affine_p is 0. Setting affine_p may be useful for quickly
        # disabling affine transformation
        if self.affine_p == 0:
            return Xb, yb

        idx = get_random_idx(Xb, self.affine_p)
        Xb_transformed = Xb.copy()

        for i in idx:
            scale = choice(self.affine_scale_choices)
            rotation = choice(self.affine_rotation_choices)
            shear = choice(self.affine_shear_choices)
            translation_y = choice(self.affine_translation_choices)
            translation_x = choice(self.affine_translation_choices)
            img_transformed, tform = im_affine_transform(
                Xb[i], return_tform=True,
                scale=scale, rotation=rotation,
                shear=shear,
                translation_y=translation_y,
                translation_x=translation_x
            )
            Xb_transformed[i] = img_transformed

        return Xb_transformed, yb


class RandomCropBatchIteratorMixin(object):
    """
    Randomly crop the image to the desired size
    """
    def __init__(self, crop_size, *args, **kwargs):
        super(RandomCropBatchIteratorMixin, self).__init__(*args, **kwargs)
        self.crop_size = crop_size

    def transform(self, Xb, yb):
        Xb, yb = super(RandomCropBatchIteratorMixin, self).transform(Xb, yb)
        # TODO:10 raise exception if Xb size is smaller than crop size
        batch_size = min(self.batch_size, Xb.shape[0])
        img_h = Xb.shape[2]
        img_w = Xb.shape[3]
        Xb_transformed = np.empty((batch_size, Xb.shape[1],
                                   self.crop_size[0], self.crop_size[1]), dtype=np.float32)
        # TODO:50 vectorize implementation if possible
        for i in range(batch_size):
            start_0 = np.random.choice(img_h - self.crop_size[0])
            end_0 = start_0 + self.crop_size[0]
            start_1 = np.random.choice(img_w - self.crop_size[1])
            end_1 = start_1 + self.crop_size[1]
            Xb_transformed[i] = Xb[i][:, start_0:end_0, start_1:end_1]
        return Xb_transformed, yb


class RandomFlipBatchIteratorMixin(object):
    """
    Randomly flip the random horizontally or vertically
    """
    def __init__(self, flip_horizontal_p=0.5, flip_vertical_p=0.5, *args, **kwargs):
        super(RandomFlipBatchIteratorMixin, self).__init__(*args, **kwargs)
        self.flip_horizontal_p = flip_horizontal_p
        self.flip_vertical_p = flip_vertical_p

    def transform(self, Xb, yb):
        Xb, yb = super(RandomFlipBatchIteratorMixin, self).transform(Xb, yb)
        Xb_flipped = Xb.copy()

        if self.flip_horizontal_p > 0:
            horizontal_flip_idx = get_random_idx(Xb, self.flip_horizontal_p)
            Xb_flipped[horizontal_flip_idx] = Xb_flipped[horizontal_flip_idx, :, :, ::-1]

        if self.flip_vertical_p > 0:
            vertical_flip_idx = get_random_idx(Xb, self.flip_vertical_p)
            Xb_flipped[vertical_flip_idx] = Xb_flipped[vertical_flip_idx, :, ::-1, :]

        return Xb_flipped, yb


class ReadImageBatchIteratorMixin(object):
    """
    Read images by file name
    """
    def __init__(self, read_image_size, read_image_prefix_path='',
                 read_image_as_gray=False, read_image_as_bc01=True,
                 read_image_as_float32=True,
                 *args, **kwargs):
        super(ReadImageBatchIteratorMixin, self).__init__(*args, **kwargs)
        self.read_image_size = read_image_size
        self.read_image_prefix_path = read_image_prefix_path
        self.read_image_as_gray = read_image_as_gray
        self.read_image_as_bc01 = read_image_as_bc01
        self.read_image_as_float32 = read_image_as_float32

    def transform(self, Xb, yb):
        Xb, yb = super(ReadImageBatchIteratorMixin, self).transform(Xb, yb)

        batch_size = min(Xb.shape[0], self.batch_size)
        num_channels = 1 if self.read_image_as_gray is True else 3
        h = self.read_image_size[0]
        w = self.read_image_size[1]

        imgs = np.empty((batch_size, num_channels, h, w), dtype=np.float32)
        for i, path in enumerate(Xb):
            img_fname = os.path.join(self.read_image_prefix_path, path)
            if self.verbose > 2:
                print('Reading %s' % img_fname)
            img = imread(img_fname,
                         as_grey=self.read_image_as_gray)

            if img.shape[0] != h or img.shape[1] != w:
                img = resize(img, (h, w))
            else:
                img = img.astype(float) / 255

            # When reading image as color image, convert grayscale image to RGB for consistency
            if len(img.shape) == 2 and self.read_image_as_gray is False:
                img = gray2rgb(img)
            if img.shape[2] == 4 and self.read_image_as_gray is False:
                img = rgba2rgb(img)

            # Transpose to bc01
            if self.read_image_as_bc01 and self.read_image_as_gray is False:
                img = img.transpose(2, 0, 1)
            elif self.read_image_as_bc01 and self.read_image_as_gray is True:
                img = np.expand_dims(img, axis=0)

            imgs[i] = img
        return imgs, yb


class MeanSubtractBatchiteratorMixin(object):
    """
    Subtract training examples by the given mean
    """
    def __init__(self, mean=None, *args, **kwargs):
        super(MeanSubtractBatchiteratorMixin, self).__init__(*args, **kwargs)
        self.mean = mean

    def transform(self, Xb, yb):
        Xb, yb = super(MeanSubtractBatchiteratorMixin, self).transform(Xb, yb)
        Xb = Xb - self.mean
        return Xb, yb


class EqualizeHistBatchIteratorMixin(object):
    """
    Simple HIstorgram Equalization. Applied per channel
    """
    def __init__(self, *args, **kwargs):
        super(EqualizeHistBatchIteratorMixin, self).__init__(*args, **kwargs)

    def transform(self, Xb, yb):
        Xb, yb = super(EqualizeHistBatchIteratorMixin, self).transform(Xb, yb)
        Xb_transformed = np.asarray([
            [equalize_hist(img_ch) for img_ch in img] for img in Xb
        ])
        Xb_transformed = Xb_transformed.astype(Xb.dtype)
        return Xb_transformed, yb


class AdjustGammaBatchIteratorMixin(object):
    """
    Brightness permutation
    """
    def __init__(self, adjust_gamma_p, adjust_gamma_chocies=[1.], *args, **kwargs):
        super(AdjustGammaBatchIteratorMixin, self).__init__(*args, **kwargs)
        self.adjust_gamma_p = adjust_gamma_p
        self.adjust_gamma_chocies = adjust_gamma_chocies

    def transform(self, Xb, yb):
        Xb, yb = super(AdjustGammaBatchIteratorMixin, self).transform(Xb, yb)
        Xb_transformed = Xb.copy()

        if self.adjust_gamma_p > 0:
            random_idx = get_random_idx(Xb, self.adjust_gamma_p)
            for i in random_idx:
                gamma = choice(self.adjust_gamma_chocies)
                Xb_transformed[i] = adjust_gamma(
                    Xb[i].transpose(1, 2, 0), gamma=gamma
                ).transpose(2, 0, 1)

        return Xb_transformed, yb


class LCNBatchIteratorMixin(object):
    """
    Apply local contrast normalization to images
    """
    def __init__(self, lcn_selem=disk(5), *args, **kwargs):
        super(LCNBatchIteratorMixin, self).__init__(*args, **kwargs)
        self.lcn_selem = lcn_selem

    def transform(self, Xb, yb):
        Xb, yb = super(LCNBatchIteratorMixin, self).transform(Xb, yb)
        Xb_transformed = np.asarray([
            local_contrast_normalization(x, selem=self.lcn_selem)
            for x in Xb
        ])
        return Xb_transformed, yb


class EqualizeAdaptHistBatchIteratorMixin(object):
    """
    Apply adaptive histogram equalization
    http://scikit-image.org/docs/dev/api/skimage.exposure.html#skimage.exposure.equalize_adapthist
    """
    def __init__(self, eqadapthist_ntiles_x=8, eqadapthist_ntiles_y=8,
                 eqadapthist_clip_limit=0.01, eqadapthist_nbins=256, *args, **kwargs):
        super(EqualizeAdaptHistBatchIteratorMixin, self).__init__(*args, **kwargs)
        self.eqadapthist_ntiles_x = eqadapthist_ntiles_x
        self.eqadapthist_ntiles_y = eqadapthist_ntiles_y
        self.eqadapthist_clip_limit = eqadapthist_clip_limit
        self.eqadapthist_nbins = eqadapthist_nbins

    def transform(self, Xb, yb):
        Xb, yb = super(EqualizeAdaptHistBatchIteratorMixin, self).transform(Xb, yb)
        # TODO:0 doesn't work for greyscale image
        Xb_transformed = np.asarray([
            equalize_adapthist(img, ntiles_x=self.eqadapthist_ntiles_x,
                               ntiles_y=self.eqadapthist_ntiles_y,
                               clip_limit=self.eqadapthist_clip_limit,
                               nbins=self.eqadapthist_nbins)
            for img in Xb.transpose(0, 2, 3, 1)])
        # Back from b01c to bc01
        Xb_transformed = Xb_transformed.transpose(0, 3, 1, 2).astype(np.float32)
        return Xb_transformed, yb


def make_iterator(name, mixin):
    """
    Return an Iterator class added with the provided mixin
    """
    mixin = [BaseBatchIterator] + mixin
    # Reverse the order for type()
    mixin.reverse()
    return type(name, tuple(mixin), {})


def make_buffer_for_iterator(source_gen, buffer_size=2):
    """
    Taken from https://github.com/benanne/kaggle-ndsb/blob/05275ce473f0516f5b0abaac6a7a08a3cefda1e8/buffering.py#L31
    Generator that runs a slow source generator in a separate thread. Beware of the GIL!
    buffer_size: the maximal number of items to pre-generate (length of the buffer)
    """
    if buffer_size < 2:
        raise RuntimeError("Minimal buffer size is 2!")

    buffer = Queue.Queue(maxsize=buffer_size - 1)
    # the effective buffer size is one less, because the generation process
    # will generate one extra element and block until there is room in the buffer.

    def _buffered_generation_thread(source_gen, buffer):
        for data in source_gen:
            buffer.put(data, block=True)
        buffer.put(None)  # sentinel: signal the end of the iterator

    thread = threading.Thread(target=_buffered_generation_thread, args=(source_gen, buffer))
    thread.daemon = True
    thread.start()

    for data in iter(buffer.get, None):
        yield data


def make_buffer_for_iterator_with_thread(gen, n_workers, buffer_size):
    wait_time = 0.02
    generator_queue = Queue.Queue()
    _stop = threading.Event()

    def generator_task():
        while not _stop.is_set():
            try:
                if generator_queue.qsize() < buffer_size:
                    generator_output = next(gen)
                    generator_queue.put(generator_output)
                else:
                    time.sleep(wait_time)
            except (StopIteration, KeyboardInterrupt), e:
                _stop.set()
                return

    generator_threads = [threading.Thread(target=generator_task) for _ in range(n_workers)]
    for thread in generator_threads:
        thread.start()

    while not _stop.is_set() or not generator_queue.empty():
        if not generator_queue.empty():
            yield generator_queue.get()
        else:
            time.sleep(wait_time)


def pmap(func, arr, n_jobs=1, *args, **kwargs):
    return Parallel(n_jobs)(delayed(func)(x, *args, **kwargs) for x in arr)


def shuffle(*arrays):
    p = np.random.permutation(len(arrays[0]))
    return [array[p] for array in arrays]


def im_affine_transform(img, scale, rotation, shear, translation_y, translation_x, return_tform=False):
    # Assumed img in c01. Convert to 01c for skimage
    img = img.transpose(1, 2, 0)
    # Normalize so that the param acts more like im_rotate, im_translate etc
    scale = 1 / scale
    translation_x = - translation_x
    translation_y = - translation_y

    # shift to center first so that image is rotated around center
    center_shift = np.array((img.shape[0], img.shape[1])) / 2. - 0.5
    tform_center = SimilarityTransform(translation=-center_shift)
    tform_uncenter = SimilarityTransform(translation=center_shift)

    rotation = np.deg2rad(rotation)
    tform = AffineTransform(scale=(scale, scale), rotation=rotation,
                            shear=shear,
                            translation=(translation_x, translation_y))
    tform = tform_center + tform + tform_uncenter

    warped_img = warp(img, tform)

    # Convert back from 01c to c01
    warped_img = warped_img.transpose(2, 0, 1)
    warped_img = warped_img.astype(img.dtype)
    if return_tform:
        return warped_img, tform
    else:
        return warped_img


def local_contrast_normalization(img, selem=disk(5)):
    img = (img * 255).astype(np.uint8)
    if len(img.shape) <= 2:
        img = rank.equalize(img, selem)
    else:
        img = np.asarray([rank.equalize(ch, selem) for ch in img])
    img = img.astype(np.float32) / 255
    return img


def get_random_idx(arr, p):
    n = arr.shape[0]
    idx = choice(n, int(n * p), replace=False)
    return idx


def write_temp_log(str):
    sys.stdout.write('\r%s' % str)
    sys.stdout.flush()
