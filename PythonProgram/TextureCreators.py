import noise
import math
from PIL import Image, ImageOps, ImageChops
import time
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
        printed_texture = printed_texture
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
