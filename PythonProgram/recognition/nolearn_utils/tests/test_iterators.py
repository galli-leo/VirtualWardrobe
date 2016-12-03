import numpy as np
import os
import pytest
from skimage.data import lena as get_lena
from skimage.io import imsave
from tempfile import mkstemp

from nolearn_utils.iterators import BaseBatchIterator, make_iterator


@pytest.fixture
def X():
    return np.random.rand(1000, 3, 48, 48)


@pytest.fixture
def y():
    return np.random.rand(1000)


@pytest.fixture
def lena_tmp_paths(n=100):
    paths = []
    for i in xrange(10):
        _, fname = mkstemp(suffix='.png')
        imsave(fname, get_lena())
        paths.append(fname)
    return np.asarray(paths)


def test_base_batch_iterator(X, y):
    iterator = BaseBatchIterator(batch_size=128)
    batches = list(iterator(X, y))
    assert len(batches) == 8
    for Xb, yb in batches[:-1]:
        assert Xb.shape[0] == 128
        assert yb.shape[0] == 128
    assert batches[-1][0].shape[0] == 104
    assert batches[-1][1].shape[0] == 104


def test_shuffle_batch_iterator(X, y):
    from nolearn_utils.iterators import ShuffleBatchIteratorMixin

    class Iterator(ShuffleBatchIteratorMixin, BaseBatchIterator):
        pass

    iterator = Iterator(batch_size=128)
    np.random.seed(42)  # Deterministic tests
    Xb, yb = iter(iterator(X, y)).next()
    assert np.all(Xb != X[:128])
    assert np.all(yb != y[:128])


def test_afffine_batch_iterator(X, y):
    # TODO:20 simple smoke test
    from nolearn_utils.iterators import AffineTransformBatchIteratorMixin

    class Iterator(AffineTransformBatchIteratorMixin, BaseBatchIterator):
        pass

    iterator = Iterator(batch_size=128, affine_p=0.5,
                        affine_scale_choices=np.linspace(0.9, 1.1, 5),
                        affine_translation_choices=np.arange(-5, 6, 10),
                        affine_rotation_choices=np.arange(0, 360))

    for Xb, yb in iterator(X, y):
        pass


def test_random_crop_batch_iterator(X, y):
    # TODO:30 simple smoke test
    from nolearn_utils.iterators import RandomCropBatchIteratorMixin

    class Iterator(RandomCropBatchIteratorMixin, BaseBatchIterator):
        pass

    iterator = Iterator(batch_size=128, crop_size=(20, 20))

    for Xb, yb in iterator(X, y):
        pass


def test_random_flip_batch_iterator(X, y):
    from nolearn_utils.iterators import RandomFlipBatchIteratorMixin
    Iterator = make_iterator('Iterator', [RandomFlipBatchIteratorMixin])
    iterator = Iterator(batch_size=128, flip_horizontal_p=0.5, flip_vertical_p=0.5)

    for Xb, yb in iterator(X, y):
        pass


def test_read_image_batch_iterator(lena_tmp_paths, y):
    # TODO:40 simple smoke test
    from nolearn_utils.iterators import ReadImageBatchIteratorMixin

    class Iterator(ReadImageBatchIteratorMixin, BaseBatchIterator):
        pass

    iterator = Iterator(batch_size=128, read_image_size=(30, 30))

    for Xb, yb in iterator(lena_tmp_paths, y):
        pass

    [os.remove(path) for path in lena_tmp_paths]


def test_mean_subtraction_batch_iterator(X, y):
    from nolearn_utils.iterators import MeanSubtractBatchiteratorMixin

    class Iterator(MeanSubtractBatchiteratorMixin, BaseBatchIterator):
        pass

    iterator = Iterator(batch_size=2, mean=np.zeros((3, 48, 48)))

    for Xb, yb in iterator(X, y):
        pass
