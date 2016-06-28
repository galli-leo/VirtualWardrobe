MIN_CONFIDENCE = 80
import random

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
        return [100, self.category]

class TshirtRecognizer(CategoryRecognizer):
    def __init__(self, *args, **kwargs):
        super(TshirtRecognizer, self).__init__(*args, **kwargs)
        self.category = "tshirt"

    def calculateConfidence(self, front, joints):
        return [random.randint(60,100), self.category]

if __name__ == "__main__":
    print(CategoryRecognizer().recognizeCategory(None, None))
