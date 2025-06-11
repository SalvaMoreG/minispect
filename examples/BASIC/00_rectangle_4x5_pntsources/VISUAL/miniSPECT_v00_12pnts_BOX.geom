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
:ROTM RM90X 90. 0. 0.
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

:P N_MODULES_IN_SCANNER_X 10			// 
:P N_MODULES_IN_SCANNER_Y  5			// 
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
:P SIZE_COLL_HOLE_RADIUS_IN 0.25
:P SIZE_COLL_HOLE_RADIUS_OUT 0.45

:P N_COLL_HOLES_X $N_MODULES_IN_SCANNER_X*$N_VOXELS_X
:P N_COLL_HOLES_Y $N_MODULES_IN_SCANNER_Y*$N_VOXELS_Y
:P N_COLL_HOLES_Z 1

:P DISTANCE_COLL_CENTRE_Z  30.0*mm
:P DISTANCE_COLL_B_SCANNER_F 2*mm // from collimator_back to scanner_front
:P DISTANCE_COLL_C_SCANNER_C $DISTANCE_COLL_B_SCANNER_F+$SIZE_COLL_Z/2.+$SIZE_SCANNER_Z/2.0

:P DISTANCE_SCANNER_CENTRE_Z $DISTANCE_COLL_CENTRE_Z+$DISTANCE_COLL_C_SCANNER_C

//
// ===================== STARTING TO BUILD THE GEOMETRY

// define collimator and holes
:VOLU collimator BOX $SIZE_COLL_X/2 $SIZE_COLL_Y/2 $SIZE_COLL_Z/2. G4_W
:PLACE collimator 1 world RM0 0. 0. $DISTANCE_COLL_CENTRE_Z

:VOLU coll_hole BOX $SIZE_COLL_HOLE_X/2 $SIZE_COLL_HOLE_Y/2 $SIZE_COLL_HOLE_Z/2. G4_AIR
//	:VOLU coll_hole CONE 0 $SIZE_COLL_HOLE_RADIUS_IN 0 $SIZE_COLL_HOLE_RADIUS_OUT $SIZE_COLL_HOLE_Z/2. G4_AIR
//		Inner radius at -fDz
//		Outer radius at -fDz
//		Inner radius at +fDz
//		Outer radius at +fDz
//		Half length in z (=fDz)

:PLACE_PARAM coll_hole 1 collimator SQUARE_XY RM0 
             $N_COLL_HOLES_X $N_COLL_HOLES_Y 
			 $SIZE_VOXEL_X $SIZE_VOXEL_Y 		
             -$SIZE_COLL_X/2.+$SIZE_VOXEL_X/2.  -$SIZE_COLL_Y/2.+$SIZE_VOXEL_Y/2.  
			// collimator hole step-size = voxel size
			// collimator hole first position is at the edge of the colimator, at the centre of the 1st voxel

// define SCANNER
//
:VOLU miniSPECT BOX $SIZE_SCANNER_X/2 $SIZE_SCANNER_Y/2 $SIZE_SCANNER_Z/2. G4_AIR
:PLACE miniSPECT 1 world RM0 0. 0. $DISTANCE_SCANNER_CENTRE_Z

:VOLU module_layer BOX $SIZE_SCANNER_X/2. $SIZE_SCANNER_Y/2. $SIZE_MODULE_Z/2. G4_AIR
:PLACE_PARAM module_layer 1 miniSPECT LINEAR_Z RM0 $N_MODULES_IN_SCANNER_Z $SIZE_MODULE_Z 
	-$SIZE_SCANNER_Z/2.+$SIZE_MODULE_Z/2.

:VOLU module BOX $SIZE_MODULE_X/2. $SIZE_MODULE_Y/2. $SIZE_MODULE_Z/2. G4_AIR
:PLACE_PARAM module 1 module_layer SQUARE_XY RM0 
                 $N_MODULES_IN_SCANNER_X $N_MODULES_IN_SCANNER_Y 
                 $SIZE_MODULE_X $SIZE_MODULE_Y
                 -$SIZE_SCANNER_X/2.+$SIZE_MODULE_X/2.  -$SIZE_SCANNER_Y/2.+$SIZE_MODULE_Y/2.

// 
// >>>>> CdTe pixels >>>>>
//
:VOLU CdTe_pixel BOX $SIZE_VOXEL_X/2.0 $SIZE_VOXEL_Y/2.0 $SIZE_VOXEL_Z/2.0 CdTe

:PLACE_PARAM CdTe_pixel 1 module SQUARE_XY RM0 
             $N_VOXELS_X $N_VOXELS_Y 
			 $SIZE_VOXEL_X $SIZE_VOXEL_Y  
             -$SIZE_MODULE_X/2.+$SIZE_VOXEL_X/2.  -$SIZE_MODULE_Y/2.+$SIZE_VOXEL_Y/2.  


//:P DUMMY $DUMMY

//
// THE SOURCE
// =================================================================

//------ Define point sources
//
:P DISTANCE_pS_pS 5.0*mm		// centre to centre
:P N_SRCs_X 5
:P N_SRCs_Y 4
:P SRC_POSITION_X_1 -1.0*(($N_SRCs_X-1)/2)*$DISTANCE_pS_pS
:P SRC_POSITION_Y_1 -1.0*(($N_SRCs_Y-1)/2)*$DISTANCE_pS_pS
:P SRC_POSITION_Z 0.0*mm
:P CUBE_SIZE 1.0*mm
:P CUBE_HALFSIZE $CUBE_SIZE/2
:P SOURCE_BOX_SIZE_X (($N_SRCs_X-1)*$DISTANCE_pS_pS)+$CUBE_SIZE
:P SOURCE_BOX_SIZE_Y (($N_SRCs_Y-1)*$DISTANCE_pS_pS)+$CUBE_SIZE
:P SOURCE_BOX_SIZE_Z $CUBE_SIZE

//	:P DUMMY $DUMMY

:VOLU virtual_src_box BOX $SOURCE_BOX_SIZE_X/2. $SOURCE_BOX_SIZE_Y/2. $SOURCE_BOX_SIZE_Z/2. G4_AIR
:PLACE virtual_src_box 1 world RM0 0.*mm 0.*mm $SRC_POSITION_Z

:VOLU source_acrylic_cube BOX $CUBE_HALFSIZE $CUBE_HALFSIZE $CUBE_HALFSIZE G4_PLEXIGLASS
:PLACE_PARAM source_acrylic_cube 1 virtual_src_box SQUARE_XY RM0 
             $N_SRCs_X $N_SRCs_Y 
			 $DISTANCE_pS_pS $DISTANCE_pS_pS
			 $SRC_POSITION_X_1 $SRC_POSITION_Y_1 
				// #x #y    #step_x #step_y    #offset_x #offset_y

:VOLU source_point_source ORB 0.1*mm G4_WATER 
:PLACE source_point_source 1 source_acrylic_cube RM0 0.  0.  0.

// ================================================================================================

//--- comment to visualize...

:VIS world OFF

:COLOUR miniSPECT     0.0 0.0 1.0 	// blue
:COLOUR module        0.0 1.0 0.0 	// green
:COLOUR CdTe_pixel    0.0 0.3 1.0	// light-blue

:COLOUR collimator    0.5 0.5 0.5 	// gray

:COLOUR source_acrylic_cube    1.0 0.0 0.0 // red

