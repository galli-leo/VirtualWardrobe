import noise
import math
from PIL import Image, ImageOps, ImageChops, ImageEnhance, ImageStat
import time
import sys
sys.path.append("E:\\Anaconda27_64\\Lib\\site-packages\\")
import cv2
import numpy as np
from Vars import LOG
import Vars
import inspect
import os

def timeit(func):
    """Times any function. Very useful for debugging slow algorithms."""
    def func_wrapper(*args, **kwargs):
        current_time = time.time()
        ret = func(*args, **kwargs)
        LOG.debug("Time taken to execute function {0}: {1} s".format(str(func.__name__), float(time.time()-current_time)))
        return ret
    return func_wrapper

class TextureCreator(object):
    """docstring for TextureCreator"""
    def __init__(self, samples, printed_texture, size = (2048, 2048), prefix="testing"):
        super(TextureCreator, self).__init__()
        self.samples = samples
        self.printed_texture = printed_texture
        self.size = size
        self.prefix = prefix
        self.num_per_processing = 1

    def save(self, image):
        """VERY HACKY, ONLY USE TO DEBUG!"""
        LOG.warning("Using hacky save function for lazy debugging.")
        try:
            os.mkdir(os.path.join(Vars.REALPATH, "image_testing",format(self.prefix)))
        except Exception as e:
            pass
        frame = inspect.currentframe()
        name = "image"
        for k,v in frame.f_back.f_locals.iteritems():
            if v is image:
                name = k
                break
        image.save(os.path.join(Vars.REALPATH, "image_testing",format(self.prefix), name+".png"))

    @timeit
    def multiply(self, rgb, mask):
        if rgb.size != mask.size:
            LOG.warning("Image mode or size not matching")
            return None
        new = Image.new(rgb.mode, rgb.size)
        m = mask.convert("L")
        w, h = new.size
        for x in range(0, w):
            for y in range(0, h):
                value = rgb.getpixel((x,y))
                if not isinstance(value, tuple):
                    value = (value,)
                value = tuple(int(i * (m.getpixel((x,y))/255.0)) for i in value)
                new.putpixel((x,y), value)

        return new

    def m(self, img, otherImage):
        new = Image.new(img.mode, img.size)
        w, h = new.size
        for x in range(0, w):
            for y in range(0, h):
                value = img.getpixel((x,y))
                otherValue = otherImage.getpixel((x,y))
                value = (otherValue*value)/255
                new.putpixel((x,y), value)

        return new

    def add(self, img, otherImage):
        if img.mode != otherImage.mode or img.size != otherImage.size:
            LOG.warning("Image mode or size not matching")
            return None
        new = Image.new(img.mode, img.size)
        w, h = new.size
        for x in range(0, w):
            for y in range(0, h):
                value = img.getpixel((x,y))
                otherValue = otherImage.getpixel((x,y))
                newValue = [0]*len(value)
                for i in range(0,len(value)):
                    newValue[i] = value[i] + otherValue[i]

                new.putpixel((x,y), tuple(newValue))

        return new

    def a(self, img, otherImage):
        if img.mode != otherImage.mode or img.size != otherImage.size:
            LOG.warning("Image mode or size not matching")
            return None
        new = Image.new(img.mode, img.size)
        w, h = new.size
        for x in range(0, w):
            for y in range(0, h):
                value = img.getpixel((x,y))
                otherValue = otherImage.getpixel((x,y))
                newValue = value + otherValue

                new.putpixel((x,y), newValue)

        return new

    def m_c(self, img, c):
        new = Image.new(img.mode, img.size)
        w, h = new.size
        for x in range(0, w):
            for y in range(0, h):
                value = img.getpixel((x,y))
                newValue = value*c

                new.putpixel((x,y), int(newValue))

        return new

    def a_c(self, img, c):
        new = Image.new(img.mode, img.size)
        w, h = new.size
        for x in range(0, w):
            for y in range(0, h):
                value = img.getpixel((x,y))
                newValue = value+c

                new.putpixel((x,y), int(newValue))

        return new



    @timeit
    def createRandomTexture(self, size, algo=noise.snoise2, octaves=26, freq=16.0, k=8, factor=4):
        img = Image.new("L", size)
        step = 256/k
        h, w = size
        for x in range(0,w):
            for y in range(0,h):
                n = algo(x/(freq*octaves), y/(freq*octaves), octaves)
                #n = abs(n*2)-1
                n *= factor
                r = int(127.0*n+128.0)

                r = int(r/float(step))*(256/(k-1))
                value = (r)
                img.putpixel((int(x),int(y)), value)

        return img

    def tileSmallerTexture(self, smaller, larger):
        s_w, s_h = smaller.size
        tiled = Image.new("RGBA", self.size)
        w, h = tiled.size
        for i in range(0, w, s_w):
            for j in range(0, h, s_h):
                #paste the image at location i, j:
                tiled.paste(smaller, (i, j))

        larger.paste(tiled, (0,0))
        return tiled

    def brightness(self,  img ):
        stat = ImageStat.Stat(img)
        r,g,b = stat.rms
        return math.sqrt(0.241*(r**2) + 0.691*(g**2) + 0.068*(b**2))

    def brightness_alt(self, img ):
        im = img.convert('L')
        stat = ImageStat.Stat(im)
        return stat.mean[0]

    def createTexture(self):
        if len(self.samples) == 0:
            LOG.warning("Not enough samples")
            return Image.new("RGBA", self.size)
        self.texture = Image.new("RGBA", self.size)
        count = 0
        images = []
        for sample in self.samples:
            img = None
            if isinstance(sample, Image.Image):
                img = sample
            else:
                img = Image.open(sample)
            count += 1
            images.append(img)
            if count >= self.num_per_processing:
                count = 0
                self.processSample(images)
                images = []

        if self.printed_texture:
            prnt = Image.open(self.printed_texture)
            prnt = prnt.crop((33, 22, 523, 534)).transpose(Image.FLIP_LEFT_RIGHT)
            new = Image.new("RGB", (512,512))
            new.paste(prnt, (11, 0))
            new.save("print_tmp.png")
            sample = self.samples[0]
            s_img = Image.open(sample)
            img1 = cv2.imread("print_tmp.png")
            img2 = cv2.imread(sample)
            bright = self.brightness_alt(s_img.convert("RGB"))
            is_white = False
            print(bright)
            if bright > 255/2:
                is_white = True
            if is_white:
                result = cv2.subtract(img2, img1)
            else:
                result = cv2.subtract(img1, img2)
            cv2.imwrite("print_tmp.png", result)
            prnt = Image.open("print_tmp.png")

            prnt = ImageEnhance.Contrast(prnt).enhance(1.5)
            if is_white:
                prnt = ImageOps.invert(prnt)
                prnt = ImageEnhance.Brightness(prnt).enhance(1.25)
            else:
                prnt = ImageEnhance.Brightness(prnt).enhance(1.75)

            prnt.save("print_tmp.png")
            prnt.save("aaa.png")
            result = cv2.imread("print_tmp.png")
            img_bw = 255*(cv2.cvtColor(result, cv2.COLOR_BGR2GRAY) > 5).astype('uint8')

            se1 = cv2.getStructuringElement(cv2.MORPH_RECT, (10,10))
            se2 = cv2.getStructuringElement(cv2.MORPH_RECT, (5,5))
            mask = cv2.morphologyEx(img_bw, cv2.MORPH_CLOSE, se1)
            mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, se2)

            mask = np.dstack([mask, mask, mask]) / 255
            out = result * mask
            cv2.imwrite("print_tmp.png", out)
            prnt = Image.open("print_tmp.png")
            self.save(prnt)
            prnt = prnt.convert("RGBA")
            w, h = prnt.size
            for y in range(0, h):
                for x in range(0, w):
                    r, g, b, a = prnt.getpixel((int(x), int(y)))
                    comb = (r + g + b) / 3.0
                    if comb < 10 and not is_white:
                        prnt.putpixel((int(x), int(y)), (0,0,0,0))
                    if comb > 245 and is_white:
                        prnt.putpixel((int(x), int(y)), (0,0,0,0))
            self.save(prnt)
            prnt = prnt.resize((460, 460), Image.ANTIALIAS).crop((11,0,523,534))
            self.texture.paste(prnt, (self.size[0]/4, 110), mask=prnt.split()[3])
            asdf = self.texture
            self.save(asdf)


        return self.texture
    def processSample(self, image):
        pass

class BigTileCreator(TextureCreator):
    """BigTileCreator. Only works for 1 sample!"""
    def processSample(self, images):
        s_image = images[0]
        self.texture = Image.new("RGBA", self.size)
        s_w, s_h = s_image.size
        big_tile = Image.new("RGBA", (s_w*2, s_h*2))
        big_tile.paste(s_image, (0,0))
        big_tile.paste(s_image.transpose(Image.FLIP_LEFT_RIGHT), (s_w,0))
        big_tile.paste(s_image.transpose(Image.FLIP_TOP_BOTTOM), (0,s_h))
        big_tile.paste(s_image.transpose(Image.FLIP_TOP_BOTTOM).transpose(Image.FLIP_LEFT_RIGHT), (s_w,s_h))
        self.tileSmallerTexture(big_tile, self.texture)

        return self.texture

class CircleCreator(TextureCreator):
    @timeit
    def processSample(self, images):
        s_image = images[0]
        self.texture = Image.new("RGBA", self.size)
        s_w, s_h = s_image.size
        orig_mask = Image.open(os.path.join(Vars.REALPATH, "mask.png")).convert("L")
        #self.save(orig_mask)
        tile = Image.new("RGBA", s_image.size)
        tile.paste(s_image, (0,0), orig_mask)
        w, h = self.size
        for i in range(0, w+s_w, s_w):
            for j in range(0, h+s_h, s_h):
                self.texture.paste(s_image, (i, j), orig_mask)
                self.texture.paste(s_image, (i-s_w/2, j-s_h/2), orig_mask)
                self.texture.paste(s_image, (i, j-s_h/2), orig_mask)
                self.texture.paste(s_image, (i-s_w/2, j), orig_mask)
        #self.save(tile)
        return self.texture

    def createMaskForOrig(self, size):
        mask = Image.new("L", size)
        for x in range(0, size[0]):
            for y in range(0, size[1]):
                f = 2
                N = (size[0])
                value = 1-math.sqrt((x-N/2)**f + (y-N/2)**f) / (N/2) * 0.5
                if value < 0:
                    value = 0
                mask.putpixel((x,y), (int(255*value),))
        self.save(mask)
        return mask

class TilableCreator(TextureCreator):
    """Borrowed from http://paulbourke.net/texture_colour/tiling/"""
    @timeit
    def processSample(self, images):
        s_image = images[0]
        s_w, s_h = s_image.size
        orig_mask = self.createMaskForOrig(s_image.size)
        mask_swapped = self.swapOppositeQuadrants(orig_mask)
        s_swapped = self.swapOppositeQuadrants(s_image)
        tile = Image.new("RGBA", s_image.size)
        tile.paste(s_image, (0,0))

        self.save(s_swapped)
        mask_swapped2 = ImageOps.invert(orig_mask)
        self.save(mask_swapped2)
        self.save(mask_swapped)
        shaded = self.m_c(mask_swapped, 0.4)
        #arch = self.m_c(self.a_c(self.m_c(mask_swapped, 1.5), -128), 2)

        mask_swapped3 = self.a(shaded, mask_swapped2)
        #mask_swapped = Image.open("testing_mask_swapped.png")
        self.save(mask_swapped3)
        orig_mask = Image.open("orig_mask.png").convert("L")

        tile.paste(s_image, (0,0), mask_swapped3)
        tile.paste(s_swapped, (0,0), orig_mask)
        self.save(tile)
        self.tileSmallerTexture(tile, self.texture)
        #self.tileSmallerTexture(s_swapped, self.texture)

        return self.texture

    def swapOppositeQuadrants(self, image):
        new = Image.new(image.mode, image.size)
        w, h = image.size
        #LOG.debug("Image size {0}".format(image.size))
        for x in range(0, w):
            for y in range(0, h):
                N = w
                new_x = (x+N/2) % N
                new_y = (y+N/2) % N
                value = image.getpixel((new_x, new_y))
                new.putpixel((x,y), value)

        return new

    def createMaskForOrig(self, size):
        mask = Image.new("L", size)
        for x in range(0, size[0]):
            for y in range(0, size[1]):
                f = 2
                N = (size[0])
                value = math.sqrt((x-N/2)**f + (y-N/2)**f) / (N/2)
                mask.putpixel((x,y), (int(255*value),))
        self.save(mask)
        return mask

class NoiseCreator(TextureCreator):
    """docstring for NoiseCreator"""
    def __init__(self, *args, **kwargs):
        super(NoiseCreator, self).__init__(*args, **kwargs)
        self.num_per_processing = 2

    @timeit
    def processSample(self, images):
        if len(images) != 2:
            LOG.warning("Wrong number of samples. Maybe you have too little?")
            return Image.new("RGBA", self.size)
        sample1 = images[0]
        s1_creator = CircleCreator([sample1], None, prefix=self.prefix)
        sample2 = images[1]
        s2_creator = CircleCreator([sample2], None, prefix=self.prefix)
        sample1 = s1_creator.createTexture()
        sample2 = s2_creator.createTexture()
        random = Image.open(os.path.join(Vars.REALPATH, "random.png"))
        rand_invert = Image.open(os.path.join(Vars.REALPATH, "random_invert.png"))
        s_w, s_h = sample1.size
        t1 = ImageChops.multiply(sample1, random.convert("RGBA"))
        t2 = ImageChops.multiply(sample2, rand_invert.convert("RGBA"))
        self.texture = ImageChops.add(t1, t2)
        return self.texture



def regenerateRandomImages():
    creator = TextureCreator([], None)
    random = creator.createRandomTexture((2048, 2048))
    random.save("random.png")
    ImageOps.invert(random).save("random_invert.png")

def runTest(klass=NoiseCreator):
    creator = klass(["back1.png", "back2.png"], None, prefix="grey")
    img = creator.createTexture()
    img.show()

if __name__ == "__main__":
    #regenerateRandomImages()
    runTest()
