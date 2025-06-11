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

:P N_MODULES_IN_SCANNER_X  5			// 
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

//
// ===================== STARTING TO BUILD THE GEOMETRY

// DEFINE Collimator ==========================================

:P COLLIMATOR_SIZE $SIZE_COLL_Z

:P N_HOLESX 2
:P N_HOLESY 2
:P DISTANCE_H_H $SIZE_SCANNER_Y/$N_HOLESY

:P DISTANCE_COLL_B_SCANNER_F 2*mm // from collimator_back to scanner_front
:P DISTANCE_COLL_B_SCANNER_B $DISTANCE_COLL_B_SCANNER_F+$SIZE_SCANNER_Z
:P DISTANCE_COLL_C_SCANNER_C $DISTANCE_COLL_B_SCANNER_F+$SIZE_COLL_Z/2.+$SIZE_SCANNER_Z/2.0

:P RMAX_COLLIMATOR_BLOCKING $SIZE_SCANNER_X

:P CONE_1_SIZE $COLLIMATOR_SIZE/2.0
:P CONE_2_SIZE $COLLIMATOR_SIZE-$CONE_1_SIZE

:P CONE_1_RMIN 0.6*mm

:P RMAX $SIZE_SCANNER_Y/$N_HOLESY*$CONE_1_SIZE/(2.0*($DISTANCE_COLL_B_SCANNER_B+$CONE_1_SIZE))+$CONE_1_RMIN

:P CONE_1_RMAX $RMAX

:P CONE_2_RMAX $RMAX
:P CONE_2_RMIN $CONE_1_RMIN

:P CONE_1_POS -$CONE_1_SIZE/2.0
:P CONE_2_POS +$CONE_2_SIZE/2.0

// Collimator position relative to source


:P HOLE_POSITION_X_1 -1.0*(($N_HOLESX-1)/2)*$DISTANCE_H_H
:P HOLE_POSITION_Y_1 -1.0*(($N_HOLESY-1)/2)*$DISTANCE_H_H

:P FOV_SIZE 5.0*cm
:P DISTANCE_COLL_F_SOURCE $CONE_1_SIZE*$FOV_SIZE/($RMAX-$CONE_1_RMIN)
:P DISTANCE_COLL_CENTRE_Z $SIZE_COLL_Z/2.0+$DISTANCE_COLL_B_SCANNER_F

:P DISTANCE_SCANNER_CENTRE_Z $DISTANCE_COLL_CENTRE_Z+$DISTANCE_COLL_C_SCANNER_C
:P COLLIMATOR_POS  $DISTANCE_COLL_CENTRE_Z

:VOLU "diabolo_0" TUBE 0 $RMAX_COLLIMATOR_BLOCKING $COLLIMATOR_SIZE/4. G4_W
:PLACE "diabolo_0"  1 world RM0  0. 0. $COLLIMATOR_POS-$SIZE_COLL_Z/4.0

:VOLU "diabolo_1" TUBE 0 $RMAX_COLLIMATOR_BLOCKING $COLLIMATOR_SIZE/4. G4_W
:PLACE "diabolo_1"  1 world RM0  0. 0. $COLLIMATOR_POS+$SIZE_COLL_Z/4.0

:VOLU "collimator_1"   CONE 0 $CONE_1_RMAX 0 $CONE_1_RMIN $CONE_1_SIZE/2. G4_AIR
:PLACE_PARAM "collimator_1"  1 "diabolo_0" SQUARE_XY RM0
                $N_HOLESX $N_HOLESY 
                $DISTANCE_H_H $DISTANCE_H_H
                $HOLE_POSITION_X_1 $HOLE_POSITION_Y_1
                


//:PLACE "collimator_1"  1 "diabolo_0"  RM0  0. 0. $CONE_1_POS

:VOLU "collimator_2"   CONE 0 $CONE_2_RMIN 0 $CONE_2_RMAX $CONE_2_SIZE/2. G4_AIR
:PLACE_PARAM "collimator_2"  1 "diabolo_1" SQUARE_XY RM0
                $N_HOLESX $N_HOLESY 
                $DISTANCE_H_H $DISTANCE_H_H
                $HOLE_POSITION_X_1 $HOLE_POSITION_Y_1
//:PLACE "collimator_2"  1 "diabolo_0"  RM0  0. 0. $CONE_2_POS


// define SCANNER ===========================================================
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


// :P DUMMY $DUMMY

//
// THE SOURCE
// =================================================================

//------ Define point sources
//
:P DISTANCE_pS_pS 10.0*mm		// centre to centre
:P N_SRCs_X 10
:P N_SRCs_Y 10
:P SRC_POSITION_X_1 -1.0*(($N_SRCs_X-1)/2)*$DISTANCE_pS_pS
:P SRC_POSITION_Y_1 -1.0*(($N_SRCs_Y-1)/2)*$DISTANCE_pS_pS
:P SRC_POSITION_Z 0.0*mm
:P CUBE_SIZE 0.25*mm
:P CUBE_HALFSIZE $CUBE_SIZE/2
:P SOURCE_BOX_SIZE_X (($N_SRCs_X-1)*$DISTANCE_pS_pS)+$CUBE_SIZE
:P SOURCE_BOX_SIZE_Y (($N_SRCs_Y-1)*$DISTANCE_pS_pS)+$CUBE_SIZE
:P SOURCE_BOX_SIZE_Z $CUBE_SIZE

// :P DUMMY $DUMMY

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

:COLOR "diabolo_0"  1 1 1
:COLOR "collimator_1"  0.0 0.0 1.0
:COLOR "collimator_2"  0.0 1.0 0.0

:COLOUR source_acrylic_cube    1.0 0.0 0.0 // red

