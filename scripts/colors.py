import os

# Colored terminal ANSI codes

def end():
    if os.name == 'posix':
        return '\033[1;m'
    else:
        return ''

def red():
    if os.name == 'posix':
        return '\033[1;31m'
    else:
        return ''

def green():
    if os.name == 'posix':
        return '\033[1;32m'
    else:
        return ''

