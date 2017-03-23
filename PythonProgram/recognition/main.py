from data import load_data, load_classes, load_g_data
import time
import json
import timeit
net = None

import matplotlib
matplotlib.use('Agg')

import numpy as np
import pandas as pd

from sklearn.cross_validation import train_test_split

import theano
try:theano.config.device = 'gpu'
except: pass

import os
from shutil import copyfile


from lasagne.layers import InputLayer, DenseLayer, DropoutLayer, FeaturePoolLayer
from lasagne.layers import Conv2DLayer, MaxPool2DLayer
from lasagne import nonlinearities
from lasagne import objectives
from lasagne import updates

from nolearn.lasagne import NeuralNet, TrainSplit
from nolearn.lasagne.handlers import SaveWeights
from nolearn.lasagne.visualize import draw_to_file, plot_occlusion, plot_conv_activity, plot_conv_weights, draw_to_notebook

from skimage.io import imread
from skimage.color import gray2rgb, rgba2rgb
from skimage.transform import resize

from nolearn_utils.iterators import (
	BufferedBatchIteratorMixin,
	ShuffleBatchIteratorMixin,
	AffineTransformBatchIteratorMixin,
	ReadImageBatchIteratorMixin,
	RandomFlipBatchIteratorMixin,
	make_iterator
)
from nolearn_utils.hooks import (
	SaveTrainingHistory, PlotTrainingHistory
)

cats = load_classes()

image_size = 224
batch_size = 128
n_classes = len(cats) #DONE:0 Determine number of classes
print(n_classes)

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
	'batch_size': batch_size,
	'read_image_size': (image_size, image_size),
	'read_image_as_gray': False,
	'read_image_prefix_path': '',
	'buffer_size': 2,
	'flip_horizontal_p': 0.5,
	'flip_vertical_p': 0,
	'affine_p': 0.5,
	'affine_scale_choices': np.linspace(0.85, 1.15, 9),
	'affine_translation_choices': np.arange(-12, 12, 1),
	'affine_rotation_choices': np.arange(-45, 50, 2.5),
}
train_iterator = TrainIterator(**train_iterator_kwargs)

test_iterator_kwargs = {
	'read_image_size': (image_size, image_size),
	'read_image_as_gray': False,
	'read_image_prefix_path': '',
	'batch_size': batch_size,
	'buffer_size': 2,
}
test_iterator = TestIterator(**test_iterator_kwargs)

save_weights = SaveWeights('model_weights.pkl', only_best=True, pickle=False)
save_training_history = SaveTrainingHistory('model_history.pkl')
plot_training_history = PlotTrainingHistory('training_history.png')
#early_stopping = EarlyStopping(metrics='valid_accuracy', patience=100, verbose=True, higher_is_better=True) BACKLOG:0 Implement early stopping
def save_to_json(nn, training_history):
	train_hist = []
	for row in training_history:
		new_row = {}
		new_row["epoch"] = row["epoch"]
		new_row["train_loss"] = row["train_loss"]
		new_row["valid_loss"] = row["valid_loss"]
		new_row["valid_accuracy"] = row["valid_accuracy"]
		new_row["max_epochs"] = nn.max_epochs
		new_row["estimate"] = (nn.max_epochs-row["epoch"])*row["dur"]
		new_row["dur"] = row["dur"]
		new_row["t_next"] = int(row["dur"])+time.time()
		new_row["t_fin"] = int(new_row["estimate"]) + time.time()
		train_hist.append(new_row)
	with open("training_history.json", "w+") as f:
		f.write(json.dumps(train_hist))

net = NeuralNet(
	layers=[
		(InputLayer, dict(name='input', shape=(None, 3, image_size, image_size))),
		(Conv2DLayer, dict(name='l1c1', num_filters=128, filter_size=(12, 12), pad=2, stride=3)),
#        (Conv2DLayer, dict(name='l1c2', num_filters=32, filter_size=(3, 3), pad='same')),
		(MaxPool2DLayer, dict(name='l1p', pool_size=3, stride=2, pad=2)),

		(Conv2DLayer, dict(name='l2c1', num_filters=192, filter_size=(6, 6), pad=2)),
#        (Conv2DLayer, dict(name='l2c2', num_filters=32, filter_size=(3, 3), pad='same')),
		(MaxPool2DLayer, dict(name='l2p', pool_size=2, stride=2, pad=0)),

		(Conv2DLayer, dict(name='l3c1', num_filters=256, filter_size=(3, 3), pad=1)),
		(Conv2DLayer, dict(name='l3c2', num_filters=256, filter_size=(3, 3), pad=1)),
		(Conv2DLayer, dict(name='l3c3', num_filters=192, filter_size=(3, 3), pad=1)),
		(MaxPool2DLayer, dict(name='l3p', pool_size=2, stride=2)),

		(DenseLayer, dict(name='l7', num_units=4096)),
#        (FeaturePoolLayer, dict(name='l7p', pool_size=2)),
		(DropoutLayer, dict(name='l7drop', p=0.5)),

		(DenseLayer, dict(name='l8', num_units=4096)),
#        (FeaturePoolLayer, dict(name='l8p', pool_size=2)),
		(DropoutLayer, dict(name='l8drop', p=0.5)),

		(DenseLayer, dict(name='out', num_units=n_classes, nonlinearity=nonlinearities.softmax)),
	],

	regression=False,
	objective_loss_function=objectives.categorical_crossentropy,

	update=updates.adam,
	update_learning_rate=0.00005,

#    update=updates.rmsprop,

	batch_iterator_train=train_iterator,
	batch_iterator_test=test_iterator,

	train_split=TrainSplit(eval_size=1./6),

	on_epoch_finished=[
		save_weights,
		save_training_history,
		plot_training_history,
		save_to_json,
#        early_stopping
	],

	verbose=10,
	max_epochs=35
)


def draw():
	global net
	net.initialize()
	draw_to_file(net, "layout.png", verbose=True)
	draw_to_notebook(net)

def train():
	global net
	X, y = load_data()
	net.fit(X, y)

	# Load the best weights from pickled model
	net.load_params_from('model_weights.pkl')

def train(percent_of_data):
	global net
	x, y = load_data()
	print(percent_of_data)
	if percent_of_data < 0.6:
		t = TrainSplit(percent_of_data)
		X, x, Y, y = t(x, y, net)
	net.fit(x, y)

def overwrite_file(f, new):
	try:
		os.remove(new)
	except Exception as e:
		print(e)
	try:
		copyfile(f, new)
	except Exception as e:
		print(e)

def rescore_iterative():
	for x in range(0,4):
		percent = "{0:.0}%".format(.125 * (2**x))
		load("iterative_training/model_weights_" + percent + ".pkl")
		print("Scoring model with " + percent +" of training data.")
		s = score()
		with open(percent + "_score.txt", "a") as p_f:
			p_f.write(str(s) + "\n")

def score_exceptions():
	global net
	X_test, y_test = load_g_data()
	cats = load_classes()
	res = net.predict(X_test)
	count = 0
	for r, x, y in zip(res, X_test, y_test):
		if r != y:
			print(x)
			copyfile(x, "exceptions/predicted_"+cats[r].name + "_actual_"+cats[y].name + "_" + str(count) + os.path.splitext(x)[1] )
			count += 1


def score_iterative():
	for x in range(0,4):
		percent = "{0:.0}%".format(.125 * (2**x))
		train(.125 * (2**x))
		load()
		print("Scoring model with " + percent +" of training data.")
		s = score()
		overwrite_file("model_weights.pkl", "iterative_training/model_weights_" + percent + ".pkl")
		overwrite_file("training_history.png", "iterative_training/training_history_" + percent + ".png")
		overwrite_file("training_history.json", "iterative_training/training_history_" + percent + ".json")
		with open(percent + "_score.txt", "a") as p_f:
			p_f.write(str(s) + "\n")

def load(f="model_weights.pkl"):
	global net
	net.load_params_from(f)

def time_me(repeats = 100):
	print("Timing...")
	times = timeit.Timer(timer_function).repeat(repeat = repeats, number=1)
	print("Minimum time taken to predict category: ", min(times))

def timer_function():
	global net
	X = np.asarray(["predicted_hoodie_confidence_0.56279528141.bmp"])
	net.predict(X)

def load_image(path):
	global net, image_size
	h, w = image_size, image_size
	img_fname = path
	img = imread(img_fname,
				 as_grey=False)
	if img.shape[0] != h or img.shape[1] != w:
		img = resize(img, (h, w))
	else:
		img = img.astype(float) / 255

	# When reading image as color image, convert grayscale image to RGB for consistency
	if len(img.shape) == 2:
		img = gray2rgb(img)
	if img.shape[2] == 4:
		img = rgba2rgb(img)

	# Transpose to bc01
	if True:
		img = img.transpose(2, 0, 1)
	return img

def predict(path, debug=False):
	global net, image_size
#    img = load_image(path)
	X = np.asarray([path])
	img = np.asarray([load_image(path)], dtype="float32")
	cats = load_classes()
	probs = net.predict_proba(X)
	if debug == True:
		for layer in net.layers:
			name = layer[0].__name__
			filename = os.path.splitext(path)[0]
			try:
				os.mkdir("debug/"+filename)
			except:
				pass
			if "2D" in name:
				real_name = layer[1]["name"]
				try:
					plt = plot_conv_activity(net.layers_[real_name], img)
					plt.savefig("debug/"+filename+"/"+real_name+"_activity.png")
					plt.close()
					#plt = plot_conv_weights(net.layers_[real_name]) DISABLED: Eats too much memory, pretty useless anyhow
					#plt.savefig("debug/"+filename+"/"+real_name+"_weights.png")
				except Exception as e:
					print(e)

	for i, x in enumerate(probs[0]):
		print(cats[i].name, x)
	result = [(cats[np.argmax(x)].id, cats[np.argmax(x)].name, x[np.argmax(x)]) for x in probs]
	return result[0]

def plot_occ(path, real):
	global net
	img = load_image(path)

	cats = load_classes()
	X = np.asarray([img], dtype="float32")
	plt = plot_occlusion(net, X, [real])
	return plt

def score():
	global net
	X_test, y_test = load_g_data()
	score = net.score(X_test, y_test)
	print("Final score of net from google images: ", score)
	return score



if __name__ == '__main__':
	draw()
	#rescore_iterative()
	#score_iterative()
	#train()
	load("iterative_training/model_weights_1e+00%.pkl")
	score()
	#score_exceptions()
	#time_me()
	#score()
