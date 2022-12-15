import itertools
from PIL import Image 

PATH = "/home/laurent/Documents/Polytech/MA2/Virtual_Reality/project/"
filename = "skybox.png"

img = Image.open(f"{PATH}{filename}")
width, height = img.size

block_size = (width//4, height//3)

for i, j in itertools.product(range(4), range(3)):
    img_res = img.crop((i*block_size[0], (j)*block_size[1], (i+1)*block_size[0], (j+1)*block_size[1]))
    if i==1 and j==0: # Top
        img_res.save(f"{PATH}top.png")
    if i==0 and j==1: # Left
        img_res.save(f"{PATH}left.png")
    if i==1 and j==1: # Center
        img_res.save(f"{PATH}front.png")
    if i==2 and j==1: # Right
        img_res.save(f"{PATH}right.png")
    if i==3 and j==1: # Right++
        img_res.save(f"{PATH}back.png")
    if i==1 and j==2: # Bottom
        img_res.save(f"{PATH}bottom.png")