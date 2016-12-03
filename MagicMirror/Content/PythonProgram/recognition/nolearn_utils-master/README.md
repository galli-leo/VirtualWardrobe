# nolearn-utils

[![Build Status](https://travis-ci.org/felixlaumon/nolearn_utils.svg?branch=master)](https://travis-ci.org/felixlaumon/nolearn_utils)

Iterators and handlers for nolearn.lasagne to allow efficient real-time image augmentation and training progress monitoring

## Real-time image augmentation

- `ShuffleBatchIteratorMixin` to shuffle training samples
- `ReadImageBatchIteratorMixin` to transform image file path into image as color or as gray, and with specified image size
- `RandomFlipBatchIteratorMixin` to randomly (uniform) flip the image horizontally or verticaly
- `AffineTransformBatchIteratorMixin` to apply affine transformation (scale, rotate, translate) to randomly selected images from the given transformation options - `BufferedBatchIteratorMixin` to perform transformation in another thread automatically and put the result in a buffer (default size = 5)
- `LCNBatchIteratorMixin` to perform local contrast normalization to images
- `MeanSubtractBatchIteratorMixin` to subtract samples from the pre-calculated mean

Example of using iterators as below:

    train_iterator_mixins = [
        ShuffleBatchIteratorMixin,
        ReadImageBatchIteratorMixin,
        RandomFlipBatchIteratorMixin,
        AffineTransformBatchIteratorMixin,
        BufferedBatchIteratorMixin,
    ]
    TrainIterator = make_iterator('TrainIterator', train_iterator_mixins)

    train_iterator_kwargs = {
        'buffer_size': 5,
        'batch_size': batch_size,
        'read_image_size': (image_size, image_size),
        'read_image_as_gray': False,
        'read_image_prefix_path': './data/train/',
        'flip_horizontal_p': 0.5,
        'flip_vertical_p': 0,
        'affine_p': 0.5,
        'affine_scale_choices': np.linspace(0.75, 1.25, 5),
        'affine_translation_choices': np.arange(-3, 4, 1),
        'affine_rotation_choices': np.arange(-45, 50, 5)
    }
    train_iterator = TrainIterator(**train_iterator_kwargs)

The `BaseBatchIterator` is also modified from `nolearn.lasagne` to provide a progress bar for training process for each iteration

## Handlers

- `EarlyStopping` stops training when loss stop improving
- `StepDecay` to gradually reduce a parameter (e.g. learning rate) over time
- `SaveTrainingHistory` to save training history (e.g. training loss)
- `PlotTrainingHistory` to plot out training loss and validation accuracy
  over time after each iteration with matplotlib

## Examples

Example code requires `scikit-learn`

### MNIST

`example/mnist/train.py` should produce a model of about 99.5% accuracy in less than 50 epoch.

MNIST data can be downloaded from
[Kaggle](https://www.kaggle.com/c/digit-recognizer).

### CIFAR10

CIFAR10 images can be downloaded from [Kaggle](https://www.kaggle.com/c/cifar-10/data). Place the downloaded data as follows:

    examples/cifar10
    ├── data
    │   ├── train
    │   |   ├── 1.png
    │   |   ├── 2.png
    │   |   ├── 3.png
    │   |   ├── ...
    │   └── trainLabels.csv
    └── train.py

`example/cifat10/train.py` should produce a model at about 85% accuracy at 100 epoch. Images are read from disk and augmented at training time (from another thread)


## TODO

- [ ] Embarrassingly parallelize transform


## License

MIT & BSD
