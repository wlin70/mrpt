	/* +---------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)               |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2016, Individual contributors, see AUTHORS file        |
   | See: http://www.mrpt.org/Authors - All rights reserved.                   |
   | Released under BSD License. See details in http://www.mrpt.org/License    |
   +---------------------------------------------------------------------------+ */

#ifndef CICPGoodnessERD_H
#define CICPGoodnessERD_H

#include <mrpt/math/CMatrix.h>
#include <mrpt/utils/CImage.h>
#include <mrpt/utils/CLoadableOptions.h>
#include <mrpt/utils/CConfigFileBase.h>
#include <mrpt/utils/CStream.h>
#include <mrpt/utils/types_simple.h>
#include <mrpt/utils/TColor.h>
#include <mrpt/obs/CObservation2DRangeScan.h>
#include <mrpt/obs/CObservation3DRangeScan.h>
#include <mrpt/obs/CRawlog.h>
#include <mrpt/opengl/CDisk.h>
#include <mrpt/opengl/CSetOfObjects.h> // TODO - is it needed?
#include <mrpt/opengl/CRenderizable.h>
#include <mrpt/opengl/CPlanarLaserScan.h>
#include <mrpt/opengl/COpenGLViewport.h>
#include <mrpt/slam/CICP.h>
#include <mrpt/system/os.h>

#include <map>
#include <string>
#include <stdlib.h> // abs

#include "CEdgeRegistrationDecider.h"

#include <iostream>

// TODO - remove these
using namespace mrpt;
using namespace mrpt::synch;
using namespace mrpt::poses;
using namespace mrpt::obs;
using namespace mrpt::system;
using namespace mrpt::graphs;
using namespace mrpt::math;
using namespace mrpt::utils;
using namespace mrpt::gui;
using namespace mrpt::opengl;
using namespace mrpt::slam;
using namespace mrpt::maps;



namespace mrpt { namespace graphslam { namespace deciders {

	/**
	 * Map type: 2D
	 * MRPT rawlog format: #1, #2
	 * Observations: CObservation2DRangeScan, CObservation3DRangeScan
	 * Edge Registration Strategy: Goodnesss threshold
	 *
 	 * Register new edges in the graph with the last added node. Criterion for
	 * adding new nodes should  be the goodness of the potential ICP edge. The
	 * nodes for ICP should be picked based on the distance from the last
	 * inserted node.
	 */
	template<
		  class GRAPH_t=typename mrpt::graphs::CNetworkOfPoses2DInf >
		class CICPGoodnessERD_t :
			public mrpt::graphslam::deciders::CEdgeRegistrationDecider_t<GRAPH_t> {
	  		public:
					typedef typename GRAPH_t::constraint_t constraint_t;
					// type of underlying poses (2D/3D)
					typedef typename GRAPH_t::constraint_t::type_value pose_t; 

					// Public methods
					//////////////////////////////////////////////////////////////
	    		CICPGoodnessERD_t();
	    		~CICPGoodnessERD_t();

					void updateDeciderState(
							mrpt::obs::CActionCollectionPtr action,
							mrpt::obs::CSensoryFramePtr observations,
							mrpt::obs::CObservationPtr observation );


					void setGraphPtr(GRAPH_t* graph);
					void setRawlogFname(const std::string& rawlog_fname);
					void setCDisplayWindowPtr(mrpt::gui::CDisplayWindow3D* win);
    			void setWindowManagerPtr(mrpt::gui::CWindowManager_t* win_manager);
    			void getEdgesStats(
    					std::map<const std::string, int>* edge_types_to_nums);

    			void initializeVisuals();
    			void updateVisuals();
    			bool justInsertedLoopClosure();

    			struct TParams: public mrpt::utils::CLoadableOptions {
    				public:
    					TParams();
    					~TParams();

    					void loadFromConfigFile(
    							const mrpt::utils::CConfigFileBase &source,
    							const std::string &section);
							void 	dumpToTextStream(mrpt::utils::CStream &out) const;

							mrpt::slam::CICP icp;
 							// maximum distance for checking other nodes for ICP constraints
							double ICP_max_distance;
							// threshold for accepting an ICP constraint in the graph
							double ICP_goodness_thresh;
							int LC_min_nodeid_diff;
							bool visualize_laser_scans;
							bool enable_intensity_viewport;
							bool enable_range_viewport;
							std::string scans_img_external_dir;

							bool has_read_config;

    			};


					// Public variables
					// ////////////////////////////
    			TParams params;

	  		private:
					// Private functions
					//////////////////////////////////////////////////////////////
					/**
		 	 	 	 * Initialization function to be called from the various constructors
		 	 	 	 */
					void initCICPGoodnessERD_t();
					void checkRegistrationCondition(
							const std::set<mrpt::utils::TNodeID>& nodes_set);
    			void registerNewEdge(
    					const mrpt::utils::TNodeID& from, 
    					const mrpt::utils::TNodeID& to,
    					const constraint_t& rel_edge );
					void checkIfInvalidDataset(mrpt::obs::CActionCollectionPtr action,
							mrpt::obs::CSensoryFramePtr observations,
							mrpt::obs::CObservationPtr observation );
    			/**
    			 * Run ICP for the laser scans of the given nodeIDs and fill the
    			 * potential constraint between them. Returns the goodness of the
    			 * potential constraint so that the caller can decide if they want to
    			 * keep the constraint or not. Returns 0 if laser scans have not been
    			 * registered at one of the give node positions
    			 */
					double getICPEdge(
							const mrpt::utils::TNodeID& from,
							const mrpt::utils::TNodeID& to,
							constraint_t* rel_edge );
					/**
					 * fill list of nodes that are within certain distance from a given
					 * node
					 */
		 			void getNearbyNodesOf(
		 					std::set<mrpt::utils::TNodeID> *nodes_set, 
							const mrpt::utils::TNodeID& cur_nodeID,
							double distance );
					/**
					 * Method that updates the m_last_laser_scan2D variable given a
					 * CObservation3DRangeScan acquired from an RGB-D camera
					 */
					void convert3DTo2DRangeScan(
							/*from = */ CObservation3DRangeScanPtr& scan3D_in,
							/*to   = */ CObservation2DRangeScanPtr* scan2D_out=NULL);
					/**
					 * In case of 3DScan images, method sets the path of each image
					 * either to ${rawlog_path_wo_extension}_Images/img_name (default
					 * choice) or to the  param.scans_img_external_storage directory specified
					 * by the user in the .ini configuration file.
					 */
					void correct3DScanImageFname(
							mrpt::utils::CImage* img,
							std::string extension = ".png");

					// Private variables
					//////////////////////////////////////////////////////////////

					GRAPH_t* m_graph;
					mrpt::gui::CDisplayWindow3D* m_win;
					mrpt::gui::CWindowManager_t* m_win_manager;

					std::string m_rawlog_fname;

					bool m_initialized_visuals;
					bool m_initialized_rgbd_viewports;
					bool m_just_inserted_loop_closure;
					bool m_contains_scans3D;

					mrpt::utils::TColorf m_search_disk_color; // see Ctor for initialization
					double m_offset_y_search_disk;
					int m_text_index_search_disk;
					

					std::map<const mrpt::utils::TNodeID, 
						mrpt::obs::CObservation2DRangeScanPtr> m_nodes_to_laser_scans;
					std::map<const std::string, int> m_edge_types_to_nums;

    			int m_last_total_num_of_nodes;
    			CObservation2DRangeScanPtr m_last_laser_scan2D;
    			CObservation3DRangeScanPtr m_last_laser_scan3D;

					// find out if decider is invalid for the given dataset
					bool m_checked_for_usuable_dataset;
					size_t m_consecutive_invalid_format_instances;
					const size_t m_consecutive_invalid_format_instances_thres;

					// TODO - Remove these
					// 3D=>2D scan conversion parameters
					const string kConversionSensorLabel;
					const double kConversionAngleSup;
					const double kConversionAngleInf;
					const double kConversionOversamplingRatio;
			};

} } } // end of namespaces

#include "CICPGoodnessERD_impl.h"
#endif /* end of include guard: CICPGoodnessERD_H */

