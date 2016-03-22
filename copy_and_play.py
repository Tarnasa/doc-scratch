import sys
import os
import pexpect
import subprocess

n = 1
while True:
    name = "build/client{0}.exe".format(n)
    try:
        os.remove(name)
        print("Replacing {0}".format(name))
    except OSError as e:
        if e.errno == os.errno.ENOENT:
            print("Creating new file {0}".format(name))
        else:
            n += 1
            continue
    os.system("cp build/client.exe {0}".format(name))
    break

al = open('a{0}.log'.format(n), 'w')
ae = open('a{0}.err'.format(n), 'w')
bl = open('b{0}.log'.format(n), 'w')
be = open('b{0}.err'.format(n), 'w')
a = subprocess.Popen("{0} Chess -s r99acm.device.mst.edu -r \"{0}\"".format(name).split(' '),
        stdout=al, stderr=ae)
b = subprocess.Popen("{0} Chess -s r99acm.device.mst.edu -r \"{0}\"".format(name).split(' '),
        stdout=bl, stderr=be)
a.wait()
b.wait()

