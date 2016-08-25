import TextureRecognizers
import random

class LongShirt(TextureRecognizers.CategoryRecognizer):
    def calculateConfidence(self, front, joints):
        return (90, "LongShirt")
