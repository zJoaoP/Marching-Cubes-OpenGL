import numpy as np
import subprocess
import pyautogui
import psutil
import time
import cv2

def takeScreenShot(imageName, low, high):
	ss = pyautogui.screenshot()
	img = np.array(ss)
	img = img[:, :, ::-1].copy()

	img = img[low[0]:low[0] + high[0], low[1]:low[1] + high[1]]
	cv2.imwrite("./Images/{}.png".format(imageName), img)

def runMarchingCubesProgram(cubeSize):
	subprocess.Popen(["./CG2", "bun_zipper.xyz", "lut.txt", "{0:2f}".format(value)])

def closeMarchingCubesProgram():
	for proc in psutil.process_iter():
		if proc.name() == "CG2":
			proc.kill()

if __name__ == "__main__":
	value = 1.0
	decay = 2
	i = 68
	while value > 0.01:
		value = 1 * pow(1 - 0.06, i)
		runMarchingCubesProgram(value)
		time.sleep(20)
		takeScreenShot(str(i), (80, 80), (630, 650))
		time.sleep(0.5)
		closeMarchingCubesProgram()
		print("{0:2f}".format(value))
		i += 1

	print(i)
