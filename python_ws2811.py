import pyautogui

print('Press Ctrl-C to quit.')
try:
	while True:
		x, y = pyautogui.position()
		pixelColor = pyautogui.screenshot().getpixel((x, y))
		positionStr += ' RGB: (' + str(pixelColor[0]).rjust(3)
		positionStr += ', ' + str(pixelColor[1]).rjust(3)
		positionStr += ', ' + str(pixelColor[2]).rjust(3) + ')'
		print(positionStr, end='')
except KeyboardInterrupt:
	print('\nDone.')