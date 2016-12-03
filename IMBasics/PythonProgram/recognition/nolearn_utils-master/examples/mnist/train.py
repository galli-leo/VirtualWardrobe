import matplotlib
matplotlib.use('Agg')

import numpy as np
import pandas as pd

from sklearn.cross_validation import train_test_split

from lasagne.layers import InputLayer, DenseLayer, DropoutLayer, FeaturePoolLayer
from lasagne.layers.dnn import Conv2DDNNLayer, MaxPool2DDNNLayer
from lasagne import nonlinearities
from lasagne import objectives
from lasagne import updates

from nolearn.lasagne import NeuralNet
from nolearn.lasagne.handlers import SaveWeights

from nolearn_utils.iterators import (
    BufferedBatchIteratorMixin,
    ShuffleBatchIteratorMixin,
    AffineTransformBatchIteratorMixin,
    make_iterator
)
from nolearn_utils.hooks import (
    SaveTrainingHistory, PlotTrainingHistory,
    EarlyStopping
)


def load_data(test_size=0.25, random_state=None):
    df = pd.read_csv('examples/mnist/train.csv')
    X = df[df.columns[1:]].values.reshape(-1, 1, 28, 28).astype(np.float32)
    X = X / 255
    y = df['label'].values.astype(np.int32)
    return train_test_split(X, y, test_size=test_size, random_state=random_state)


image_size = 28
batch_size = 1024
n_classes = 10

train_iterator_mixins = [
    ShuffleBatchIteratorMixin,
    AffineTransformBatchIteratorMixin,
    BufferedBatchIteratorMixin,
]
TrainIterator = make_iterator('TrainIterator', train_iterator_mixins)

test_iterator_mixins = [
    BufferedBatchIteratorMixin,
]
TestIterator = make_iterator('TestIterator', test_iterator_mixins)

train_iterator_kwargs = {
    'batch_size': batch_size,
    'buffer_size': 5,
    'affine_p': 0.5,
    'affine_scale_choices': np.linspace(0.75, 1.25, 5),
    'affine_translation_choices': np.arange(-5, 6, 1),
    'affine_rotation_choices': np.arange(-45, 50, 5),
}
train_iterator = TrainIterator(**train_iterator_kwargs)

test_iterator_kwargs = {
    'batch_size': batch_size,
    'buffer_size': 5,
}
test_iterator = TestIterator(**test_iterator_kwargs)

save_weights = SaveWeights('./examples/mnist/model_weights.pkl', only_best=True, pickle=False)
save_training_history = SaveTrainingHistory('./examples/mnist/model_history.pkl')
plot_training_history = PlotTrainingHistory('./examples/mnist/training_history.png')
early_stopping = EarlyStopping(metrics='valid_accuracy', patience=100, verbose=True, higher_is_better=True)

net = NeuralNet(
    layers=[
        (InputLayer, dict(name='in', shape=(None, 1, image_size, image_size))),
        (Conv2DDNNLayer, dict(name='l1c1', num_filters=32, filter_size=(3, 3), pad='same')),
        (Conv2DDNNLayer, dict(name='l1c2', num_filters=32, filter_size=(3, 3), pad='same')),
        (MaxPool2DDNNLayer, dict(name='l1p', pool_size=3, stride=2)),

        (Conv2DDNNLayer, dict(name='l2c1', num_filters=32, filter_size=(3, 3), pad='same')),
        (Conv2DDNNLayer, dict(name='l2c2', num_filters=32, filter_size=(3, 3), pad='same')),
        (MaxPool2DDNNLayer, dict(name='l2p', pool_size=3, stride=2)),

        (DenseLayer, dict(name='l7', num_units=256)),
        (FeaturePoolLayer, dict(name='l7p', pool_size=2)),
        (DropoutLayer, dict(name='l7drop', p=0.5)),

        (DenseLayer, dict(name='l8', num_units=256)),
        (FeaturePoolLayer, dict(name='l8p', pool_size=2)),
        (DropoutLayer, dict(name='l8drop', p=0.5)),

        (DenseLayer, dict(name='out', num_units=10, nonlinearity=nonlinearities.softmax)),
    ],

    regression=False,
    objective_loss_function=objectives.categorical_crossentropy,

    update=updates.adam,
    update_learning_rate=1e-3,

    batch_iterator_train=train_iterator,
    batch_iterator_test=test_iterator,

    on_epoch_finished=[
        save_weights,
        save_training_history,
        plot_training_history,
        early_stopping
    ],

    verbose=10,
    max_epochs=100
)

if __name__ == '__main__':
    X_train, X_test, y_train, y_test = load_data(test_size=0.25, random_state=42)
    net.fit(X_train, y_train)

    # Load the best weights from pickled model
    net.load_params_from('./examples/mnist/model_weights.pkl')

    score = net.score(X_test, y_test)
    print 'Final score %.4f' % score
