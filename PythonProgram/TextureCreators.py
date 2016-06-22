import noise
import math
from PIL import Image, ImageOps

class TextureCreator(object):
    """docstring for TextureCreator"""
    def __init__(self, samples, printed_texture, size = (2048, 2048)):
        super(TextureCreator, self).__init__()
        self.samples = samples
        printed_texture = printed_texture
        self.size = size

    def createRandomTexture(self, size, algo=noise.snoise2):
        img = Image.new("L", size)
        h, w = size
        for x in range(0,w-1):
            for y in range(0,h-1):
                n = algo(x*0.45, y*0.67)
                r = abs(int(255*n))
                value = (r)
                img.putpixel((int(x),int(y)), value)

        return img

    def tileSmallerTexture(self, smaller, larger):
        s_w, s_h = samller.size
        tiled = Image.new("RGBA", self.size)
        w, h = tiled.size
        for i in range(0, w, s_w):
            for j in range(0, h, s_h):
                #paste the image at location i, j:
                tiled.paste(s_image, (i, j))

        larger.paste(tiled, (0,0))
        return tiled

    def createTexture(self):
        self.texture = Image.new("RGBA", self.size)
        for path in self.samples:
            s_image = Image.open(path)
            tiled = self.tileSmallerTexture(s_image, self.texture)
            tiled.save("tiled.png")
            tiled_flipped = tiled.transpose(Image.FLIP_LEFT_RIGHT)
            tiled_flipped2 = tiled_flipped.transpose(Image.FLIP_TOP_BOTTOM)

        return self.texture

class BigTileCreator(TextureCreator):
    """docstring for BigTileCreator"""
    def createTexture(self):
        if len(self.samples) == 0:
            print("Not enough samples")
            return Image.new("RGBA", self.size)
        self.texture = Image.new("RGBA", self.size)
        for path in self.samples:
            s_image = Image.open(path)
            s_w, s_h = s_image.size
            big_tile = Image.new("RGBA", (s_w*2, s_h*2))
            big_tile.paste(s_image, (0,0))
            big_tile.paste(s_image.transpose(Image.FLIP_LEFT_RIGHT), (s_w,0))
            big_tile.paste(s_image.transpose(Image.FLIP_TOP_BOTTOM), (0,s_h))
            big_tile.paste(s_image.transpose(Image.FLIP_TOP_BOTTOM).transpose(Image.FLIP_LEFT_RIGHT), (s_w,s_h))
            self.tileSmallerTexture(big_tile, self.texture)

        return self.texture

class TilingPrevention(TextureCreator):
    """Borrowed from http://paulbourke.net/texture_colour/tiling/"""
    def createTexture(self):
        self.texture = Image.new("RGBA", self.size)
        for path in self.samples:
            s_image = Image.open(path)
            s_w, s_h = s_image.size
            orig_mask = self.createMaskForOrig(s_image.size)
            mask_swapped = self.swapOppositeQuadrants(orig_mask)
            s_swapped = self.swapOppositeQuadrants(s_image)
            tile = Image.new("RGBA", s_image.size)
            tile.paste(s_image, (0,0), orig_mask)
            tile.paste(s_swapped, (0,0), mask_swapped)
            self.tileSmallerTexture(tile, self.texture)

        return self.texture

    def swapOppositeQuadrants(self, image):
        new = Image.new(image.mode, image.size)
        w, h = image.size
        for x in range(0, w):
            for y in range(0, h):
                N = w
                new_x = (x+N/2) % N
                new_y = (y+N/2) % N
                value = image.getpixel((new_x, new_y))
                new.putpixel((x,y), value)

    def createMaskForOrig(self, size):
        mask = Image.new("L", size)
        for x in range(0, size[0]):
            for y in range(0, size[1]):
                N = (size[0])
                value = math.sqrt((x-N/2)**2 + (y-N/2)**2) / (N/2)
                mask.putpixel((x,y), (int(255*value),))

        print("Hello")
        mask.save("testing_mask.png")
        return mask




if __name__ == "__main__":
    creator = TextureCreator([], None)
    img = creator.createRandomTexture(creator.size)
    img.save("random.png")
    invert = ImageOps.invert(img)
    invert.save("random_invert.png")
