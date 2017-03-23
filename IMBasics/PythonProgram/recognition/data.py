import sys
import os
import import_im
import DBWrappers
from DBWrappers import ClothingItem, Category
import Vars
import DatabaseInterface
import numpy as np
from fix_images import delete_broken_images
from skimage.io import imread
from skimage.color import gray2rgb, rgba2rgb
from skimage.transform import resize
from nolearn.lasagne import NeuralNet, TrainSplit

def load_images_from_path(path):
	imgs = []
	valid_images = [".jpg",".gif",".png",".tga", ".jpeg"]
	for f in os.listdir(path):
		ext = os.path.splitext(f)[1]
		if ext.lower() not in valid_images:
			continue
		imgs.append(os.path.join(path,f))
	return imgs

def split_data():
	X, y = load_data()


def load_data():
	print("Loading data...")
	X, y = np.asarray([], dtype="float32"), np.asarray([], dtype="int32")
	cats = load_classes()
	for cat in cats:
		delete_broken_images(cat.name)
		imgs = load_images_from_path(cat.name)
		X = np.concatenate((X,imgs))
		y = np.concatenate((y, [cats.index(cat)]*len(imgs)))
		print("Loaded {0} images for category {1}".format(len(imgs), cat.name))
	print("Loaded data.")
	return X,y

def load_data_into_memory(h, w):
	X, y = load_data()
	imgs = np.asarray([], dtype="float32")
	print("Loading images into memory...")
	for i, path in enumerate(X):
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
		elif False:
			img = np.expand_dims(img, axis=0)

		imgs = np.append(imgs, img)

	print("Loaded Images into memory.")
	return imgs, y


def load_g_data():
	print("Loading data...")
	X, y = np.asarray([], dtype="float32"), np.asarray([], dtype="int32")
	cats = load_classes()
	for cat in cats:
		try:
			path = "g_" + cat.name
			delete_broken_images(path)
			imgs = load_images_from_path(path)
			X = np.concatenate((X,imgs))
			y = np.concatenate((y, [cats.index(cat)]*len(imgs)))
		except:
			print("Fuck you")
	print("Loaded data.")
	return X,y

def load_classes():
	path = import_im.find_python_path()
	Vars.setRealPath(path)
	DBWrappers.loadCategories()
	cats = Vars.CATEGORIES
	cats = cats[1:len(cats)]
	cats = [x for x in cats if x.name != ""]
	return cats

def fix_data():
	cats = load_classes()
	for cat in cats:
		delete_broken_images(cat.name)


if __name__ == "__main__":
	X, y = load_data()
	cats = load_classes()
	print(X[0], y[0])
	print(len(X), len(y))
	print(X[1500], y[len(y)-3])
