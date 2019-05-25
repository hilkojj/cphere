
for side in front right left top bottom back
do
    echo $side | cowsay
    ~/Compressonator_Linux_x86_64_3.1.307/CompressonatorCLI ./assets/textures/space_cubemap/$side.png ./assets/textures/space_cubemap/$side.dds -miplevels 0 -fd DXT3 | lolcat
done
