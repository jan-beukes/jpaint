HEADER=../src/images.h

echo "#ifndef IMAGES_H" > $HEADER
echo "#define IMAGES_H" >> $HEADER
echo "" >> $HEADER

for file in *.png; do
    xxd -i "$file" >> $HEADER # xxd goated
done

echo "#endif // IMAGES_H" >> $HEADER