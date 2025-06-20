# miniSPECT
The code is not thought to be compiled outside the IFAE's network and machines. 
In the envir_mkfls the general methodology to compile the code can be seen,
but it is not expected to be performed in a random machine. 

This is how the code is expected to be compiled. 
Example: if you are on machine VIP12:
	cd miniSPECT 		(the main directory)
	cp envirs_mkfls/envir_VIP12.sh .
	cp envirs_mkfls/Makefile_VIP12 Makefile
	source envir_VIP12.sh 
	mkdir tmp bin
	make

If you are willing to compile the code using an external machine, you have to create your 
own .sh and makefile to compile, as well as downloading and installing ROOT (https://root.cern/install/).
----------------------------------------------------------------------
If you want to start a simulation, you have to download GAMOS (https://fismed.ciemat.es/GAMOS/gamos_download.php),
go inside any of the examples subdirectories and execute "gamos miniSPECT_gamma.in". This will start a simulation, 
which will result in a .root file with the raw data obtained. 

To filter the data you have to use "source doConvert.sh", where you will need to have compiled the code "ListModeDataProcessing".
This will generate a LM file, which name has to be written in the minispect_parameters.conf. 

After that, the analysis can be performed simply by executing miniSpectator in the same example subdirectory, where the 
configuration file can also be found. The desired analysis can be changed by editing the configuration file.  
