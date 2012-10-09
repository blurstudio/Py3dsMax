
import sys
import os
import subprocess


def main():
	call_children()


def call_children():
	rootdir = os.path.normpath(os.path.dirname(os.path.abspath(__file__)))
	for root, dirnames, filenames in os.walk(rootdir):
		if root == rootdir:
			continue
		for fn in filenames:
			if fn == 'rst_maker.py':
				fp = os.path.join(root, fn)
				subprocess.call('python "%s"' % fp)


if __name__ == '__main__':
	main()
