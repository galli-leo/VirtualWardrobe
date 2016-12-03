import matplotlib
matplotlib.use('Agg')

import os
import numpy as np
import pandas as pd

from lasagne.layers import InputLayer, DenseLayer, DropoutLayer, FeaturePoolLayer
from lasagne.layers.dnn import Conv2DDNNLayer, MaxPool2DDNNLayer
from lasagne import nonlinearities
from lasagne import objectives
from lasagne import updates

from nolearn.lasagne import NeuralNet
from nolearn.lasagne.handlers import SaveWeights

from sklearn.cross_validation import StratifiedKFold
from sklearn.preprocessing import LabelEncoder

from nolearn_utils.iterators import (
    BufferedBatchIteratorMixin,
    ReadImageBatchIteratorMixin,
    ShuffleBatchIteratorMixin,
    RandomFlipBatchIteratorMixin,
    AffineTransformBatchIteratorMixin,
    make_iterator
)
from nolearn_utils.hooks import SaveTrainingHistory, PlotTrainingHistory


def stratified_train_test_split(X, y, test_size=0.25, random_state=None):
    n_folds = int(1 / test_size)
    skf = StratifiedKFold(y, n_folds=n_folds, random_state=random_state)
    train_idx, test_idx = iter(skf).next()
    return X[train_idx], X[test_idx], y[train_idx], y[test_idx]


def load_data(test_size=0.25, random_state=None, data_dir='./examples/cifar10/data'):
    csv_fname = os.path.join(data_dir, 'trainLabels.csv')
    df = pd.read_csv(csv_fname)
    X = df['id'].apply(lambda i: '%s.png' % i).values
    y = LabelEncoder().fit_transform(df['label'].values)

    X_train, X_test, y_train, y_test = stratified_train_test_split(X, y, test_size=test_size, random_state=random_state)
    y_train, y_test = y_train.astype(np.int32), y_test.astype(np.int32)
    return X_train, X_test, y_train, y_test

batch_size = 32
n_classes = 10
image_size = 32

train_iterator_mixins = [
    ShuffleBatchIteratorMixin,
    ReadImageBatchIteratorMixin,
    RandomFlipBatchIteratorMixin,
    AffineTransformBatchIteratorMixin,
    BufferedBatchIteratorMixin,
]
TrainIterator = make_iterator('TrainIterator', train_iterator_mixins)

test_iterator_mixins = [
    ReadImageBatchIteratorMixin,
    BufferedBatchIteratorMixin,
]
TestIterator = make_iterator('TestIterator', test_iterator_mixins)

train_iterator_kwargs = {
    'buffer_size': 5,
    'batch_size': batch_size,
    'read_image_size': (image_size, image_size),
    'read_image_as_gray': False,
    'read_image_prefix_path': './examples/cifar10/data/train/',
    'flip_horizontal_p': 0.5,
    'flip_vertical_p': 0,
    'affine_p': 0.5,
    'affine_scale_choices': np.linspace(0.75, 1.25, 5),
    'affine_shear_choices': np.linspace(0.75, 1.25, 5),
    'affine_translation_choices': np.arange(-3, 4, 1),
    'affine_rotation_choices': np.arange(-45, 50, 5)
}
train_iterator = TrainIterator(**train_iterator_kwargs)

test_iterator_kwargs = {
    'buffer_size': 5,
    'batch_size': batch_size,
    'read_image_size': (image_size, image_size),
    'read_image_as_gray': False,
    'read_image_prefix_path': './examples/cifar10/data/train/',
}
test_iterator = TestIterator(**test_iterator_kwargs)

save_weights = SaveWeights('./examples/cifar10/model_weights.pkl', only_best=True, pickle=False)
save_training_history = SaveTrainingHistory('./examples/cifar10/model_history.pkl')
plot_training_history = PlotTrainingHistory('./examples/cifar10/training_history.png')

net = NeuralNet(
    layers=[
        (InputLayer, dict(name='in', shape=(None, 3, image_size, image_size))),

        (Conv2DDNNLayer, dict(name='l1c1', num_filters=16, filter_size=(3, 3), pad='same')),
        (Conv2DDNNLayer, dict(name='l1c2', num_filters=16, filter_size=(3, 3), pad='same')),
        (Conv2DDNNLayer, dict(name='l1c3', num_filters=32, filter_size=(3, 3), pad='same')),
        (Conv2DDNNLayer, dict(name='l1c4', num_filters=32, filter_size=(3, 3), pad='same')),
        (MaxPool2DDNNLayer, dict(name='l1p', pool_size=3, stride=2)),

        (Conv2DDNNLayer, dict(name='l2c1', num_filters=32, filter_size=(3, 3), pad='same')),
        (Conv2DDNNLayer, dict(name='l2c2', num_filters=32, filter_size=(3, 3), pad='same')),
        (Conv2DDNNLayer, dict(name='l2c3', num_filters=64, filter_size=(3, 3), pad='same')),
        (Conv2DDNNLayer, dict(name='l2c4', num_filters=64, filter_size=(3, 3), pad='same')),
        (MaxPool2DDNNLayer, dict(name='l2p', pool_size=3, stride=2)),

        (DenseLayer, dict(name='l7', num_units=512)),
        (FeaturePoolLayer, dict(name='l7p', pool_size=2)),
        (DropoutLayer, dict(name='l7drop', p=0.5)),

        (DenseLayer, dict(name='l8', num_units=512)),
        (FeaturePoolLayer, dict(name='l8p', pool_size=2)),
        (DropoutLayer, dict(name='l8drop', p=0.5)),

        (DenseLayer, dict(name='out', num_units=n_classes, nonlinearity=nonlinearities.softmax)),
    ],

    regression=False,
    objective_loss_function=objectives.categorical_crossentropy,

    update=updates.adam,

    batch_iterator_train=train_iterator,
    batch_iterator_test=test_iterator,

    on_epoch_finished=[
        save_weights,
        save_training_history,
        plot_training_history
    ],

    verbose=10,
    max_epochs=250,
)


if __name__ == '__main__':
    # X_train, X_test are image file names
    # They will be read in the iterator
    X_train, X_test, y_train, y_test = load_data(test_size=0.25, random_state=42)

    net.fit(X_train, y_train)

    # Load the best weights from pickled model
    net.load_params_from('./examples/cifar10/model_weights.pkl')

    score = net.score(X_test, y_test)
    print 'Final score %.4f' % score
