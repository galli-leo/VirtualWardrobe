import numpy as np
from skimage.transform import SimilarityTransform
from skimage.transform import AffineTransform
# from skimage.transform import warp
from skimage.transform._warps_cy import _warp_fast


def warp(img, tf, output_shape, mode='constant', order=0):
    """
    This wrapper function is faster than skimage.transform.warp
    """
    m = tf.params
    img = img.transpose(2, 0, 1)
    t_img = np.zeros(img.shape, img.dtype)
    for i in range(t_img.shape[0]):
        t_img[i] = _warp_fast(img[i], m, output_shape=output_shape[:2],
                              mode=mode, order=order)
    t_img = t_img.transpose(1, 2, 0)
    return t_img


def augment_color(img, ev, u, sigma=0.1, color_vec=None):
    """
    https://github.com/sveitser/kaggle_diabetic/blob/master/data.py
    """
    if color_vec is None:
        if not sigma > 0.0:
            color_vec = np.zeros(3, dtype=np.float32)
        else:
            color_vec = np.random.normal(0.0, sigma, 3)

    alpha = color_vec.astype(np.float32) * ev
    noise = np.dot(u, alpha.T)
    return img + noise[:, np.newaxis, np.newaxis]


def im_affine_transform(img, scale, rotation, shear,
                        translation_y, translation_x, return_tform=False):
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

    warped_img = warp(img, tform, output_shape=img.shape)

    # Convert back from 01c to c01
    warped_img = warped_img.transpose(2, 0, 1)
    warped_img = warped_img.astype(img.dtype)

    if return_tform:
        return warped_img, tform
    else:
        return warped_img
