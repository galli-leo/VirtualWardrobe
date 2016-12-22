# Virtual Wardrobe
A proof of concept application of a virtual wardrobe, which is able to scan clothes
and display them on a 3D model of a character. This was achieved by combining C++,
Python as well as Unreal Engine 4. To scan the clothes a Kinect V2 is used.
A C++ class provides easy access to stored clothes and
scanning for new ones. Furthermore, a convolutional neural network was trained to recognize
which category a clothing item belongs to. It recognized the different categories very well
and - for example - had a confidence of around 92% when predicting jeans.

#Download
To download a precompiled binary version click [here](https://github.com/galli-leo/IntelligentMirror/releases/latest).

#License
All rights, titles to, ownership of and all intellectual property rights and all other proprietary rights in the Software belong to, vest in and shall remain by Leonardo Galli (2016). Exceptions include personal and educational use.
