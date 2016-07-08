MIN_CONFIDENCE = 80
import random
import cv2
import numpy
from PIL import Image
from TextureCreators import timeit

class CategoryRecognizer(object):
    """docstring for TexutreRecognizer"""
    def __init__(self):
        super(CategoryRecognizer, self).__init__()
        self.category = "uncategorized"

    def recognizeCategory(self, front, joints):
        global MIN_CONFIDENCE
        highest_confidence = None
        for klass in self.__class__.__subclasses__():
            recog = klass()
            conf = recog.recognizeCategory(front, joints)
            if highest_confidence == None:
                highest_confidence = conf
            if highest_confidence[0] < conf[0]:
                highest_confidence = conf
        if highest_confidence is not None:
            if highest_confidence[0]>=MIN_CONFIDENCE:
                return highest_confidence


        return self.calculateConfidence(front, joints)

    def calculateConfidence(self, front, joints):
        return (100, self.category)

class TshirtRecognizer(CategoryRecognizer):
    def __init__(self, *args, **kwargs):
        super(TshirtRecognizer, self).__init__(*args, **kwargs)
        self.category = "tshirt"

    def calculateConfidence(self, front, joints):
        return (random.randint(60,100), self.category)

if __name__ == "__main__":
    print(CategoryRecognizer().recognizeCategory(None, None))

class TemplateRecgonizer(object):
    def __init__(self):
        super(TemplateRecgonizer, self).__init__()

    @timeit
    def findMatch(self, snippet, clothes):
        max_value = (0.0, None)
        for cloth in clothes:
            if len(cloth.texturesamples)>0:
                template = cloth.texturesamples[0]
                cv_temp = numpy.array(Image.open(template).convert("RGB"), dtype=numpy.uint8)
                front = numpy.array(Image.open(snippet).convert("RGB"), dtype=numpy.uint8)
                # Apply template Matching
                res = cv2.matchTemplate(front,cv_temp,cv2.TM_CCOEFF_NORMED)
                min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(res)
                print(max_val)
                if max_val > max_value[0]:
                    max_value = (max_val, cloth)


        return max_value
