# import the necessary packages
from sklearn.neighbors import KNeighborsClassifier
from sklearn.cross_validation import train_test_split
from imutils import paths
import numpy as np
import import_im
import DBWrappers
from DBWrappers import ClothingItem, Category
import Vars
import DatabaseInterface
from fix_images import delete_broken_images
import data
import argparse
import imutils
import cv2
import os
import cPickle as pickle
import dill
import matplotlib.pyplot as plt
import timeit
from shutil import copyfile

model = None

def resize(image, size=(224,224)):
	return cv2.resize(image, size)

def image_to_feature_vector(image, size=(224, 224)):
	# resize the image to a fixed size, then flatten the image into
	# a list of raw pixel intensities
	return image.flatten()

def extract_color_histogram(image, bins=(8, 8, 8)):
	# extract a 3D color histogram from the HSV color space using
	# the supplied number of `bins` per channel
	hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
	hist = cv2.calcHist([hsv], [0, 1, 2], None, bins, [0, 180, 0, 256, 0, 256])

	# handle normalizing the histogram if we are using OpenCV 2.4.X
	if imutils.is_cv2():
		hist = cv2.normalize(hist)

	# otherwise, perform "in place" normalization in OpenCV 3 (I
	# personally hate the way this is done
	else:
		cv2.normalize(hist, hist)

	# return the flattened histogram as the feature vector
	return hist.flatten()

def extract_edges(image):
	gray = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
	edges = 255-cv2.Canny(gray, 50, 100)
	return edges.flatten()

def load_image(f, convert_fn = image_to_feature_vector):
	image = resize(cv2.imread(f))
	return convert_fn(image)

def load_data(converter, test = False, return_files = False):
	images = []
	labels = []
	imgs = []
	if test:
		print("Loading test data...")
	else:
		print("Loading data...")
	cats = data.load_classes()
	for cat in cats:
		print("Loading category {0}...".format(cat.name))
		data.delete_broken_images(cat.name)
		path = cat.name
		if test:
			path = "g_" + path
		valid_images = [".jpg",".gif",".png",".tga", ".jpeg"]
		for f in os.listdir(path):
			ext = os.path.splitext(f)[1]
			if ext.lower() not in valid_images:
				continue
			img = load_image(os.path.join(path, f), converter)
			imgs.append(os.path.join(path, f))
			images.append(img)
			labels.append(cat.name)
	print("Loaded data.")
	if return_files:
		return images, labels, imgs
	return images, labels

def train(converter = image_to_feature_vector, k = 3, percent_of_data = 1.0, images = None, labels = None):
	if images == None:
		images, labels = load_data(converter)
		images = np.asarray(images)
		labels = np.asarray(labels)
		print(percent_of_data)
		if percent_of_data < 0.6:
			X, images, Y, labels = train_test_split(images, labels, test_size=percent_of_data, random_state=0)
	print("[INFO] {0} matrix: {1:.2f}MB".format(converter.__name__, images.nbytes / (1024 * 1000.0)))
	print("[INFO] evaluating accuracy...")
	model = KNeighborsClassifier(n_neighbors=k, n_jobs=-1)
	model.fit(images, labels)
	return model

def train_and_get_exceptions():
	model = train()
	images, labels, files = load_data(image_to_feature_vector, True, True)
	images = np.asarray(images)
	labels = np.asarray(labels)
	count = 0
	for (image, label, f) in zip(images, labels, files):
		res = model.predict([image])
		if res[0] != label:
			print(f, res[0], label)
			copyfile(f, "exceptions_knn/predicted_"+ res[0] + "_actual_"+ label + "_" + str(count) + os.path.splitext(f)[1] )
			count += 1

def train_and_score(converter = image_to_feature_vector, k=3, percent_of_data = 1.0, images = None, labels = None):
	model = train(converter, k, percent_of_data, images, labels)
	images, labels = load_data(converter, True)
	images = np.asarray(images)
	labels = np.asarray(labels)
	print("[INFO] scoring with {0:.2f}% of data...".format(percent_of_data * 100))
	acc = model.score(images, labels)
	print("[INFO] {0} pixel accuracy: {1:.2f}%".format(converter.__name__, (acc * 100)))
	return acc

def time_iteratively(repeats = 5):
	with open("knn_iterative_timing.txt", "w") as f:
		for x in range(0,4):
			t = time_me(repeats, .125 * (2**x))
			f.write(str(t)+"\n")

def time_me(repeats = 5, percent = 1.0):
	global model
	model = train(image_to_feature_vector, 3, percent)
	print("Timing...")
	times = timeit.Timer(timer_function).repeat(repeat = repeats, number=1)
	print("Minimum time taken to predict category: ", min(times))
	return min(times)

def timer_function():
	global model
	image = image_to_feature_vector(resize(cv2.imread("predicted_hoodie_confidence_0.56279528141.bmp")))
	images = np.asarray([image])
	model.predict(images)


def t_s_iterative(converter = image_to_feature_vector, k=3):
	with open(converter.__name__ + "_iterative.txt", "w") as f:
		for x in range(0,4):
			percent = "{0:.0}%".format(.125 * (2**x))
			s = train_and_score(converter, k, .125 * (2**x))
			f.write(str(s)+"\n")

def t_s_find_k(converter = image_to_feature_vector, percent_of_data = 0.25):
	with open(converter.__name__ + "_find_k.txt", "w") as f:
		x = []
		y = []
		images, labels = load_data(converter)
		images = np.asarray(images)
		labels = np.asarray(labels)
		print(percent_of_data)
		if percent_of_data < 0.6:
			X, images, Y, labels = train_test_split(images, labels, test_size=percent_of_data, random_state=0)
		for k in range(1,15,2):
			s = train_and_score(converter, k, percent_of_data, images, labels)
			f.write(str(k) + ": " + str(s)+"\n")
			x.append(k)
			y.append(s)
			#plt.clear()
			plt.plot(x,y)
			plt.savefig(converter.__name__ + "_find_k.png")



if __name__ == "__main__":
	#t_s_iterative()
	#time_iteratively()
	#train_and_get_exceptions()
	pass
