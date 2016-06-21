import noise
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

    def createTexture(self):
        self.texture = Image.new("RGBA", self.size)
        random = self.createRandomTexture(size=self.size)
        for path in self.samples:
            print("t")
            s_image = Image.open(path)
            s_w, s_h = s_image.size
            tiled = Image.new("RGBA", self.size)
            w, h = tiled.size
            for i in range(0, w, s_w):
                for j in range(0, h, s_h):
                    # Change brightness of the images, just to emphasise they are unique copies
                    print(i,j)
                    #paste the image at location i, j:
                    tiled.paste(s_image, (i, j))

            self.texture.paste(tiled, (0,0), random)

        return self.texture

if __name__ == "__main__":
    creator = TextureCreator([], None)
    img = creator.createRandomTexture(creator.size)
    img.save("random.png")
    invert = ImageOps.invert(img)
    invert.save("random_invert.png")
