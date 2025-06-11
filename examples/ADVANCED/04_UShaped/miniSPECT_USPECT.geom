//----- Define materials
:MIXT "NIST_Polymethyl Methacrylate" 1.19 3
   H 0.080541
   C 0.599846
   O 0.319613
:MATE_MEE "NIST_Polymethyl Methacrylate" 74*eV

:MATE NIST_Graphite 6 12.0107 1.7

:MIXT CdTe 5.85 2
   G4_Cd  0.468355
   G4_Te  0.531645

//---- Define Rotation Matrices
:ROTM RM0 0. 0. 0.
:ROTM RM90XY -90. 90. 0.
:ROTM RM90Z 0. 0. 90.
:ROTM RM00 0. 0. 0.

//---- Define world
:VOLU world BOX 100*cm 100*cm 70*cm G4_AIR

//---- Geometry Parameters

:P N_VOXELS_X 10
:P N_VOXELS_Y 10
:P N_VOXELS_Z 1

:P SIZE_VOXEL_X 1*mm
:P SIZE_VOXEL_Y 1*mm
:P CdTe_THIK 2*mm
:P SIZE_VOXEL_Z $CdTe_THIK

:P SIZE_MODULE_X $N_VOXELS_X*$SIZE_VOXEL_X 
:P SIZE_MODULE_Y $N_VOXELS_Y*$SIZE_VOXEL_Y 
:P SIZE_MODULE_Z $N_VOXELS_Z*$SIZE_VOXEL_Z 

:P N_MODULES_IN_SCANNER_X  4			// 
:P N_MODULES_IN_SCANNER_Y  8			// 
:P N_MODULES_IN_SCANNER_Z  2			// 

//--- Define dimensions of general scanner
:P SIZE_SCANNER_X $N_MODULES_IN_SCANNER_X*$SIZE_MODULE_X 
:P SIZE_SCANNER_Y $N_MODULES_IN_SCANNER_Y*$SIZE_MODULE_Y 
:P SIZE_SCANNER_Z $N_MODULES_IN_SCANNER_Z*$SIZE_MODULE_Z 

//--- Define dimensions of collimator
:P SIZE_COLL_X $SIZE_SCANNER_X
:P SIZE_COLL_Y $SIZE_SCANNER_Y
:P SIZE_COLL_Z 20.0*mm

//--- Define dimensions of collimator-holes (supposing box-shaped...)
:P SIZE_COLL_HOLE_X 0.9
:P SIZE_COLL_HOLE_Y 0.9
:P SIZE_COLL_HOLE_Z $SIZE_COLL_Z

:P N_COLL_HOLES_X $N_VOXELS_X
:P N_COLL_HOLES_Y $N_VOXELS_Y

:P DISTANCE_SCANNER_CENTRE_Z $SIZE_SCANNER_Z/2
:P DISTANCE_COLL_B_SCANNER_F 2*mm // from collimator_back to scanner_front

:P DISTANCE_COLL_CENTRE_Z $SIZE_COLL_Z/2+$SIZE_SCANNER_Z+$DISTANCE_COLL_B_SCANNER_F
:P DISTANCE_COLL_CENTRE_Z_SIDES $SIZE_COLL_Z+$SIZE_SCANNER_Z+$DISTANCE_COLL_B_SCANNER_F+$SIZE_COLL_X/4
:P DISTANCE_COLL_SIDES $SIZE_COLL_X/2+$SIZE_COLL_Z/2

:P DISTANCE_SCANNER_SIDES $DISTANCE_COLL_SIDES+$SIZE_SCANNER_Z/2+$DISTANCE_COLL_B_SCANNER_F+$SIZE_COLL_Z/2

//
// ===================== STARTING TO BUILD THE GEOMETRY

// define collimator and holes
:VOLU collimator_b BOX $SIZE_COLL_X/2 $SIZE_COLL_Y/2 $SIZE_COLL_Z/2 G4_AIR
:VOLU collimator_s BOX $SIZE_COLL_Z/2 $SIZE_COLL_Y/2 $SIZE_COLL_X/4 G4_AIR

:PLACE collimator_b 1 world RM0 0. 0. $DISTANCE_COLL_CENTRE_Z
:PLACE collimator_s 1 world RM0 -$DISTANCE_COLL_SIDES 0. $DISTANCE_COLL_CENTRE_Z_SIDES
:PLACE collimator_s 2 world RM0 $DISTANCE_COLL_SIDES 0. $DISTANCE_COLL_CENTRE_Z_SIDES

:VOLU collimator_module_b BOX $SIZE_MODULE_X/2. $SIZE_MODULE_Y/2. $SIZE_COLL_Z/2. G4_W
:VOLU collimator_module_s BOX $SIZE_COLL_Z/2. $SIZE_MODULE_Y/2. $SIZE_MODULE_X/2. G4_W

:PLACE_PARAM collimator_module_b 1 collimator_b SQUARE_XY RM0 
                 $N_MODULES_IN_SCANNER_X $N_MODULES_IN_SCANNER_Y 
                 $SIZE_MODULE_X $SIZE_MODULE_Y
                 -$SIZE_COLL_X/2.+$SIZE_MODULE_X/2.  -$SIZE_COLL_Y/2.+$SIZE_MODULE_Y/2.
                 
:PLACE_PARAM collimator_module_s 1 collimator_s SQUARE_YZ RM0 
                 $N_MODULES_IN_SCANNER_Y $N_MODULES_IN_SCANNER_X/2
                 $SIZE_MODULE_Y $SIZE_MODULE_X
                 -$SIZE_SCANNER_Y/2.+$SIZE_MODULE_Y/2. -$SIZE_SCANNER_X/4.+$SIZE_MODULE_X/2. 
                 
:VOLU coll_hole_b BOX $SIZE_COLL_HOLE_X/2 $SIZE_COLL_HOLE_Y/2 $SIZE_COLL_HOLE_Z/2 G4_AIR
:VOLU coll_hole_s BOX $SIZE_COLL_HOLE_Z/2 $SIZE_COLL_HOLE_Y/2 $SIZE_COLL_HOLE_X/2 G4_AIR

:PLACE_PARAM coll_hole_b 1 collimator_module_b SQUARE_XY RM0 
             $N_COLL_HOLES_X $N_COLL_HOLES_Y 
			 $SIZE_VOXEL_X $SIZE_VOXEL_Y 		
             -$SIZE_MODULE_X/2.+$SIZE_VOXEL_X/2.  -$SIZE_MODULE_Y/2.+$SIZE_VOXEL_Y/2.  
             
:PLACE_PARAM coll_hole_s 1 collimator_module_s SQUARE_YZ RM0 
             $N_COLL_HOLES_Y $N_COLL_HOLES_X
			 $SIZE_VOXEL_Y $SIZE_VOXEL_X 		
             -$SIZE_MODULE_Y/2.+$SIZE_VOXEL_Y/2. -$SIZE_MODULE_X/2.+$SIZE_VOXEL_X/2. 

// define SCANNER
//
:VOLU miniSPECT_b BOX $SIZE_SCANNER_X/2 $SIZE_SCANNER_Y/2 $SIZE_SCANNER_Z/2 G4_AIR
:VOLU miniSPECT_s BOX $SIZE_SCANNER_Z/2 $SIZE_SCANNER_Y/2 $SIZE_SCANNER_X/4 G4_AIR

:PLACE miniSPECT_b 1 world RM0 0. 0. $DISTANCE_SCANNER_CENTRE_Z
:PLACE miniSPECT_s 1 world RM0 -$DISTANCE_SCANNER_SIDES 0. $DISTANCE_COLL_CENTRE_Z_SIDES
:PLACE miniSPECT_s 2 world RM0 $DISTANCE_SCANNER_SIDES 0. $DISTANCE_COLL_CENTRE_Z_SIDES

:VOLU module_layer_b BOX $SIZE_SCANNER_X/2. $SIZE_SCANNER_Y/2. $SIZE_MODULE_Z/2 G4_AIR
:VOLU module_layer_s BOX $SIZE_MODULE_Z/2. $SIZE_SCANNER_Y/2. $SIZE_SCANNER_X/4 G4_AIR

:PLACE_PARAM module_layer_b 1 miniSPECT_b LINEAR_Z RM0 $N_MODULES_IN_SCANNER_Z $SIZE_MODULE_Z 
	-$SIZE_SCANNER_Z/2.+$SIZE_MODULE_Z/2.
	
:PLACE_PARAM module_layer_s 1 miniSPECT_s LINEAR_X RM0 $N_MODULES_IN_SCANNER_Z $SIZE_MODULE_Z 
    -$SIZE_SCANNER_Z/2.+$SIZE_MODULE_Z/2.

:VOLU module_b BOX $SIZE_MODULE_X/2. $SIZE_MODULE_Y/2. $SIZE_MODULE_Z/2. G4_AIR
:VOLU module_s BOX $SIZE_MODULE_Z/2. $SIZE_MODULE_Y/2. $SIZE_MODULE_X/2. G4_AIR

:PLACE_PARAM module_b 1 module_layer_b SQUARE_XY RM0 
                 $N_MODULES_IN_SCANNER_X $N_MODULES_IN_SCANNER_Y 
                 $SIZE_MODULE_X $SIZE_MODULE_Y
                 -$SIZE_SCANNER_X/2.+$SIZE_MODULE_X/2.  -$SIZE_SCANNER_Y/2.+$SIZE_MODULE_Y/2.
                 
:PLACE_PARAM module_s 1 module_layer_s SQUARE_YZ RM0 
                 $N_MODULES_IN_SCANNER_Y $N_MODULES_IN_SCANNER_X/2
                 $SIZE_MODULE_Y $SIZE_MODULE_X
                 -$SIZE_SCANNER_Y/2.+$SIZE_MODULE_Y/2. -$SIZE_SCANNER_X/4.+$SIZE_MODULE_X/2. 

// 
// >>>>> CdTe pixels >>>>>
//
:VOLU CdTe_pixel_b BOX $SIZE_VOXEL_X/2.0 $SIZE_VOXEL_Y/2.0 $SIZE_VOXEL_Z/2.0 CdTe
:VOLU CdTe_pixel_s BOX $SIZE_VOXEL_Z/2.0 $SIZE_VOXEL_Y/2.0 $SIZE_VOXEL_X/2.0 CdTe

:PLACE_PARAM CdTe_pixel_b 1 module_b SQUARE_XY RM0 
             $N_VOXELS_X $N_VOXELS_Y 
			 $SIZE_VOXEL_X $SIZE_VOXEL_Y  
             -$SIZE_MODULE_X/2.+$SIZE_VOXEL_X/2.  -$SIZE_MODULE_Y/2.+$SIZE_VOXEL_Y/2.  

:PLACE_PARAM CdTe_pixel_s 1 module_s SQUARE_YZ RM0 
             $N_VOXELS_Y $N_VOXELS_X
			 $SIZE_VOXEL_Y $SIZE_VOXEL_X
             -$SIZE_MODULE_Y/2.+$SIZE_VOXEL_Y/2. -$SIZE_MODULE_X/2.+$SIZE_VOXEL_X/2.
             
//
// THE SOURCE
// =================================================================
//------ Define point sources
//
:P DISTANCE_pS_pS 10.0*mm		// centre to centre
:P N_SRCs_X 2
:P N_SRCs_Y 2
:P SRC_POSITION_X_1 -1.0*(($N_SRCs_X-1)/2)*$DISTANCE_pS_pS
:P SRC_POSITION_Y_1 -1.0*(($N_SRCs_Y-1)/2)*$DISTANCE_pS_pS
:P SRC_POSITION_Z 36.0*mm
:P CUBE_SIZE 0.25*mm
:P CUBE_HALFSIZE $CUBE_SIZE/2
:P SOURCE_BOX_SIZE_X (($N_SRCs_X-1)*$DISTANCE_pS_pS)+$CUBE_SIZE
:P SOURCE_BOX_SIZE_Y (($N_SRCs_Y-1)*$DISTANCE_pS_pS)+$CUBE_SIZE
:P SOURCE_BOX_SIZE_Z $CUBE_SIZE


:VOLU virtual_src_box BOX $SOURCE_BOX_SIZE_X/2. $SOURCE_BOX_SIZE_Y/2. $SOURCE_BOX_SIZE_Z/2. G4_AIR
:PLACE virtual_src_box 1 world RM0 0.*mm 0.*mm $SRC_POSITION_Z

:VOLU source_acrylic_cube BOX $CUBE_HALFSIZE $CUBE_HALFSIZE $CUBE_HALFSIZE G4_PLEXIGLASS
:PLACE source_acrylic_cube 1 virtual_src_box RM0 0. 0. 0.

:VOLU source_point_source ORB 0.1*mm G4_WATER 
:PLACE source_point_source 1 source_acrylic_cube RM0 0. 0. 0.

// ================================================================================================

//--- comment to visualize...


:VIS world OFF

:COLOUR miniSPECT_b     0.0 0.0 1.0 	// blue
:COLOUR miniSPECT_s     0.0 0.0 1.0 	// blue

:COLOUR module_b        0.0 1.0 0.0 	// green
:COLOUR module_s        0.0 1.0 0.0 	// green

:COLOUR CdTe_pixel_b    0.0 0.3 1.0	// light-blue
:COLOUR CdTe_pixel_s    0.0 0.3 1.0	// light-blue

:COLOUR collimator_b    0.5 0.5 0.5 	// gray
:COLOUR collimator_s    0.5 0.5 0.5 	// gray

:COLOUR source_acrylic_cube    1.0 0.0 0.0 // red

