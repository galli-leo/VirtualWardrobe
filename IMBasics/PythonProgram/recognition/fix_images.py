from PIL import Image
import os

def is_image_broken(path):
    try:
        img = Image.open(path)
        img.verify()
        return False
    except:
        return True

def verify_images_in_folder(folder):
    valid_images = [".jpg",".gif",".png",".tga"]
    for f in os.listdir(folder):
        ext = os.path.splitext(f)[1]
        if ext.lower() not in valid_images:
            continue
        path = os.path.join(path,f)
        if is_image_broken(path):
            print("Found broken image at: " + path)

def delete_broken_images(folder):
    valid_images = [".jpg",".gif",".png",".tga"]
    for f in os.listdir(folder):
        ext = os.path.splitext(f)[1]
        if ext.lower() not in valid_images:
            continue
        path = os.path.join(folder,f)
        if is_image_broken(path):
            print("Removing broken image at: " + path)
            os.remove(path)
