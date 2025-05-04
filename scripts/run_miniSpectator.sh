/bin/cp minispect_parameters.conf base_conf 

TOTNAME="Table_TOTAL.dat"
echo " " > ${TOTNAME}

SMPL="0"

cat base_conf | grep -v sampling > 	minispect_parameters.conf 
new_line="sampling    "$SMPL"     //"
echo $new_line >> minispect_parameters.conf

nohup miniSpectator

new_dir="SAMPLING_"$SMPL
image_name="image_"$SMPL".png"
sm_image_name="sm_image_"$SMPL".png"

mkdir -p ${new_dir}
image_dir="images"
mkdir -p ${image_dir}
/bin/cp image_XYobj.png ${image_dir}/${image_name}
/bin/cp image_XYsmooth.png ${image_dir}/${sm_image_name}
/bin/mv *.png ${new_dir}/.
/bin/mv Table.dat ${new_dir}/.

# Add Table contents to general Total table file (plus an extra column, with "set_$SMPL" as 1st column:
#value=$(${new_dir}/Table.dat | grep Sensitivity | awk '{ print $2 }')
value=$(grep "Sensitivity" "${new_dir}/Table.dat" | awk '{print $NF}')
echo $value

SAMPLES=""

for i in {1..4}; do
    vals=$( echo "$i*$value/5" | bc )
    SAMPLES+=$vals$'\n'
done

cat ${new_dir}/Table.dat | awk '{ print "set_" '$SMPL' "  " $0 }' 
cat ${new_dir}/Table.dat | awk '{ print "set_" '$SMPL' "  " $0 }' >> ${TOTNAME}
    
for SMPL in $SAMPLES; do
	cat base_conf | grep -v sampling > 	minispect_parameters.conf 
	new_line="sampling    "$SMPL"     //"
	echo $new_line >> minispect_parameters.conf
	
	nohup miniSpectator
	
	new_dir="SAMPLING_"$SMPL
	image_name="image_"$SMPL".png"
    sm_image_name="sm_image_"$SMPL".png"

	mkdir -p ${new_dir}
	image_dir="images"
	mkdir -p ${image_dir}
	/bin/cp image_XYobj.png ${image_dir}/${image_name}
    /bin/cp image_XYsmooth.png ${image_dir}/${sm_image_name}
	/bin/mv *.png ${new_dir}/.
	/bin/mv Table.dat ${new_dir}/.
	
    cat ${new_dir}/Table.dat | grep -v Peaks | awk '{ print "set_" '$SMPL' "  " $0 }' 
    cat ${new_dir}/Table.dat | grep -v Peaks | awk '{ print "set_" '$SMPL' "  " $0 }' >> ${TOTNAME}
done

source gif.sh

