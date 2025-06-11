
An example of how to run on PIC
===============================
Login on the PIC machine and go to this directory.
	/data/vip/common/machiel/MyWork_2025/202501_miniSPECT_gamos/PinHole_v01b

To run it yourself, you will have to copy the following files to your own directory:
	DetParameters.txt
	command_G6AndProc_PIC_CreateCondorStuff.sh
	miniSPECT_arrow.geom
	miniSPECT_gamma.in

Then do the following steps:
1. edit the following line in "command_G6AndProc_PIC_CreateCondorStuff.sh"
	nameTag=<some_name>
(Give it some name you can recognize)
2. in the terminal, type: "source command_G6AndProc_PIC_CreateCondorStuff.sh" and answer the following questions:
	Storing GmDataTTree root files? 
		no
	Which flavour? (short/long)
		long
	how many cpus?
		2
	Use Centos7 Container (type 'no' if you don't want it, anything else means 'yes')
		y
3. You will now have a new file "my_condor.sub". You can start up the process with typing: 
	condor_submit my_condor.sub

With the command condor_q you can see the progress of your job:
$ condor_q
-- Schedd: submit02.pic.es : <193.109.174.74:9618?... @ 01/13/25 17:17:02
OWNER BATCH_NAME      SUBMITTED   DONE   RUN    IDLE   HOLD  TOTAL JOB_IDS
...

When it is done, you will have a new directory on PIC, in /pnfs/pic.es/data/vip/PROCESS/ with the results. 
The main file, will be the "LM.out" file, which is an ASCII file with all hits in the scanner.


An example of how to run on your PC
====================================
Do the following steps: 

1. Run the simulation, with: 
	gamos miniSPECT_gamma.in

NOTE: there are various things you can change in the "in" file. For instance: currently, it is using a 140 keV gamma source with a fixed direction towards the scanner. You can also use Tc99m source or a F18 source (but you will loose statistics, more events will escape the scanner).

2. List the root files into a list file, with: 
	ls -1 GmDataTTree_tree_1000.root > listGAMOS.txt
2. Convert the GAMOS root file to an LM-file with: 
	source doConvert.sh 	
This will give you an LM.out file in the directory "outputs/"

Processing the LM.out file
============================
Once you have the LM.out file, you can do the following things: 

1) Run a simple ROOT script to get some fundamental images: 
 	$ root
	root [0] .L ~/VIP/trunk/miniSPECT/root/ReadLM_SPECT.C 
	root [1] Main()
	give list mode (LM) hits filename

The root script plots an E spectrum and an XY view. The XY view you can adjust interactively.

2) Process the LM.out file in some "fancy" way, using "miniSpectator" (~/VIP/trunk/miniSPECT/bin/miniSpectator).
For this, you need two parameter files:
	- minispect_parameters.conf
	- minispect_fov_parameters.conf
The files in this example already have the correct contents, but feel to change things and see what happens

Now, you can run "miniSpectator:
	~/VIP/trunk/miniSPECT/bin/miniSpectator

(Or do the following: 
	- edit ~/bin/setALL_PATHS.sh (I think in your case, this script calls setSoftwarePath.sh, so: 
		- edit ~/bin/setSoftwarePath.sh
		- add "export miniSPECT_PATH=${SOFTHOME}/miniSPECT/bin/" somewhere
		- edit the path line: 
		export PATH=$PATH:$VIPCommonPATH:$OSEM_PATH:(etc...):${miniSPECT_PATH}
	- source ~/bin/setALL_PATHS.sh
	- Now, you can use simply: miniSpectator   )
	
The program has the following output files:
	image_E.png
	image_lineprofiles.png
	image_XY.png
	spect_image.img_NEW
In the case of a rectangle 5x4 phantom, it also writes some useful information on the screen.

The spect_image.img_NEW file is an image file that can be read by AMIDE.  I always rename it like this: 
	mv spect_image.img_NEW spect_image.img

In AMIDE, you have to open it like this:
	$ amide	 (lower letters, no capitals)
This will open the AMIDE window. A Manual on how to use AMIDE is given in the main examples folder...










