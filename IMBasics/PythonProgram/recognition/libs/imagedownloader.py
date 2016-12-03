import sys
import os
import time
import tarfile
import threading
import Queue
import functools

if sys.version_info >= (3,):
    import urllib.request as urllib2
    import urllib.parse as urlparse
else:
    import urllib2
    import urlparse
    import urllib

SHOULD_TERMINATE = False

#python downloadutils.py --wnid n03595614 --downloadImages

class ThreadPool(object):
    def __init__(self, max_workers):
        self.queue = Queue.Queue()
        self.workers = [threading.Thread(target=self._worker) for _ in range(max_workers)]
    def start(self):
        for worker in self.workers:
            worker.start()
    def stop(self):
        for _ in range(self.workers):
            self.queue.put(None)
        for worker in self.workers:
            worker.join()
    def submit(self, job):
        self.queue.put(job)
    def _worker(self):
        global SHOULD_TERMINATE
        while True:
            if SHOULD_TERMINATE is True:
                break
            job = self.queue.get()
            if job is None:
                break
            job()

def download_file(url, dire, desc=None, renamed_file=None):
    global SHOULD_TERMINATE
    filename = None
    try:
        u = urllib2.urlopen(url)

        scheme, netloc, path, query, fragment = urlparse.urlsplit(url)
        filename = os.path.basename(path)
        if not filename:
            filename = 'downloaded.file'

        if not renamed_file is None:
            filename = renamed_file

        if desc:
            filename = os.path.join(desc, filename)

        if dire:
            filename = os.path.join(dire, filename)

        meta = u.info()
        meta_func = meta.getheaders if hasattr(meta, 'getheaders') else meta.get_all
        meta_length = meta_func("Content-Length")

        _, ext = os.path.splitext(filename)
        good_ext = [".jpg", ".png", ".jpeg"]
        if not ext.lower() in good_ext:
            print("not an image")
            return filename
        #print(meta_length[0])
        if int(meta_length[0]) < 10000 or u.getcode() > 200:
            print("shit image", meta_length[0])
            return filename

        with open(filename, 'wb') as f:
            file_size = None
            if meta_length:
                file_size = int(meta_length[0])
            #print("Downloading: {0} Bytes: {1}".format(url, file_size))

            file_size_dl = 0
            block_sz = 8192
            while True:
                buffer = u.read(block_sz)
                if not buffer:
                    break

                file_size_dl += len(buffer)
                f.write(buffer)

                status = "{0:16}".format(file_size_dl)
                if file_size:
                    status += "   [{0:6.2f}%]".format(file_size_dl * 100 / file_size)
                status += chr(13)
    except KeyboardInterrupt as e:
        SHOULD_TERMINATE = True
        print(e)
    except Exception as e:
        print(e)

    return filename

class ImageNetDownloader:
    def __init__(self):
        self.host = 'http://www.image-net.org'

    def extractTarfile(self, filename):
        tar = tarfile.open(filename)
        tar.extractall()
        tar.close()

    def downloadBBox(self, wnid):
        filename = str(wnid) + '.tar.gz'
        url = self.host + '/downloads/bbox/bbox/' + filename
        try:
            filename = self.download_file(url, self.mkWnidDir(wnid))
            currentDir = os.getcwd()
            os.chdir(wnid)
            self.extractTarfile(filename)
            print 'Download bbbox annotation from ' + url + ' to ' + filename
            os.chdir(currentDir)
        except Exception, error:
            print 'Fail to download' + url

    def getImageURLsOfWnid(self, wnid):
        url = 'http://www.image-net.org/api/text/imagenet.synset.geturls?wnid=' + str(wnid)
        f = urllib.urlopen(url)
        contents = f.read().split('\n')
        imageUrls = []

        for each_line in contents:
            # Remove unnecessary char
            each_line = each_line.replace('\r', '').strip()
            if each_line:
                imageUrls.append(each_line)

        return imageUrls

    def mkWnidDir(self, wnid):
        if not os.path.exists(wnid):
            os.mkdir(wnid)
        return os.path.abspath(wnid)

    def downloadImagesByURLs(self, wnid, imageUrls):
        # save to the dir e.g: n005555_urlimages/
        wnid_urlimages_dir = os.path.join(self.mkWnidDir(wnid), str(wnid) + '_urlimages')
        if not os.path.exists(wnid_urlimages_dir):
            os.mkdir(wnid_urlimages_dir)

        urls = imageUrls

        results = [list() for _ in urls]
        results_lock = threading.Lock()

        pool = ThreadPool(max_workers=8)
        pool.start()
        for url in urls:
            pool.submit(functools.partial(download_file, url, wnid_urlimages_dir))
        pool.stop()

    def downloadOriginalImages(self, wnid, username, accesskey):
        download_url = 'http://www.image-net.org/download/synset?wnid=%s&username=%s&accesskey=%s&release=latest&src=stanford' % (wnid, username, accesskey)
        try:
             download_file = self.download_file(download_url, self.mkWnidDir(wnid), wnid + '_original_images.tar')
        except Exception, erro:
            print 'Fail to download : ' + download_url

        currentDir = os.getcwd()
        extracted_folder = os.path.join(wnid, wnid + '_original_images')
        if not os.path.exists(extracted_folder):
            os.mkdir(extracted_folder)
        os.chdir(extracted_folder)
        self.extractTarfile(download_file)
        os.chdir(currentDir)
        print 'Extract images to ' + extracted_folder
