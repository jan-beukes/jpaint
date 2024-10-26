echo "#ifndef IMAGES_H" > images.h
echo "#define IMAGES_H" >> images.h
echo "" >> images.h

for file in *.png; do
    xxd -i "$file" >> images.h # xxd goated
done

echo "#endif // IMAGES_H" >> images.h