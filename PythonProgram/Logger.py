from termcolor import colored
import inspect
import time
import os
import datetime

def caller_name(skip=1):
    """Get a name of a caller in the format module.class.method

       `skip` specifies how many levels of stack to skip while getting caller
       name. skip=1 means "who calls me", skip=2 "who calls my caller" etc.

       An empty string is returned if skipped levels exceed stack height
    """
    stack = inspect.stack()
    start = 0 + skip
    if len(stack) < start + 1:
      return ''
    parentframe = stack[start][0]

    name = []
    module = inspect.getmodule(parentframe)
    # `modname` can be None when frame is executed directly in console
    # TODO(techtonik): consider using __main__
    if module:
        name.append(module.__name__)
    # detect classname
    if 'self' in parentframe.f_locals:
        # I don't know any way to detect call from the object method
        # XXX: there seems to be no way to detect static method call - it will
        #      be just a function call
        name.append(parentframe.f_locals['self'].__class__.__name__)
    codename = parentframe.f_code.co_name
    if codename != '<module>':  # top level usually
        name.append( codename ) # function or a method
    del parentframe
    return ":".join(name)

class Logger(object):
    """docstring for Logger"""
    def __init__(self, folder="Logs/", log_level=1, include_timestamp=False):
        super(Logger, self).__init__()
        self.folder = folder
        self.symbol_map = {"warning" : "!", "default" : "-", "critical" : "!!!", "debug" : "*"}
        self.log_levels = ["debug", "default", "warning", "critical"]
        self.log_level = log_level
        self.color_map = {"warning" : "yellow", "default" : "black", "critical" : "red", "debug" : "magenta"}
        self.add_time = include_timestamp
        self.today = datetime.datetime.now().strftime('%Y-%m-%d')
        self.openLogFile

    def getFilePath(self):
        path = os.path.join(self.folder, self.today+".txt")

    def openLogFile(self):
        self.file = open(self.getFilePath, "a")

    def log(self, message, level, skip=2):
        if self.log_levels.index(level) < self.log_level:
            return
        color = self.color_map[level]
        symbol = self.symbol_map[level]
        msg = "[{0}][{1}] {2}".format(symbol, caller_name(skip), message)
        text_colored = colored(msg, color)
        print(text_colored)

    def debug(self, message):
        self.log(message, "debug", 3)

    def default(self, message):
        self.log(message, "default", 3)

    def warning(self, message):
        self.log(message, "warning", 3)

    def critical(self, message):
        self.log(message, "critical", 3)

    def info(self, message):
        self.log(message, "default", 3)
