#ifndef SCENE_PHYSICS_DATA_FORCES
#define SCENE_PHYSICS_DATA_FORCES

#include <iostream>

#include <boost/filesystem.hpp>
#include <btBulletDynamicsCommon.h>

#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/registration/transforms.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/registration/icp.h>

// for limiting the ICP area to only include area around initial pose estimate
#include <pcl/filters/crop_box.h>
#include <pcl/filters/filter.h>
#include <pcl/features/moment_of_inertia_estimation.h>
// #include <pcl/recognition/ransac_based/trimmed_icp.h>
#include "utility.h"
#include "physics_world_parameters.h"

typedef pcl::PointCloud<pcl::PointXYZ>::Ptr PointCloudXYZPtr;
typedef pcl::PointCloud<pcl::PointXYZ> PointCloudXYZ;

// Mode for generating the feedback force based on point pair
enum FeedbackForceMode {
	// Find the closest point to each mesh point for all mesh point
	// Moderate speed
	CLOSEST_POINT, 

	// Find the ICP correspondence from the mesh to the scene points for every frame
	// Very slow
	FRAME_BY_FRAME_ICP_CORRESPONDENCE,

	// Use cached icp result for generating feedback force instead of doing icp for every frame
	// Fastest, but depends on the accuracy of the initial estimated pose
	CACHED_ICP_CORRESPONDENCE
};

class FeedbackDataForcesGenerator
{
public:
	FeedbackDataForcesGenerator();
	
	void setFeedbackForceMode(int mode);
	void applyFeedbackForces(btRigidBody &object, const std::string &model_name);
	std::pair<btVector3, btVector3> applyFeedbackForcesDebug(const btTransform &object_real_pose, const std::string &model_name);

	void setSceneData(PointCloudXYZPtr scene_data);

	// Input sampled point cloud from the mesh
	void setModelCloud(const PointCloudXYZPtr mesh_surface_sampled_cloud, const std::string &model_name);
	bool setModelDirectory(const std::string &model_directory);
	bool setModelCloud(const std::string &model_name);

	void setForcesParameter(const btScalar &forces_magnitude_per_point, const btScalar &max_point_distance_threshold);
	void resetCachedIcpResult();
	void removeCachedIcpResult(const std::string &object_id);
	void updateCachedIcpResultMap(const btRigidBody &object, 
		const std::string &model_name);
	void manualSetCachedIcpResultMapFromPose(const btRigidBody &object, 
		const std::string &model_name);
	void manualSetCachedIcpResultMapFromPose(const btTransform &object_pose,
		const std::string &object_id, const std::string &model_name);
	double getIcpConfidenceResult(const std::string &model_name, const btTransform &object_pose);

	void setDebugMode(const bool &debug_flag);
	
	PointCloudXYZPtr getTransformedObjectCloud(const std::string &model_name, const btTransform &object_real_pose) const;
	
	int force_data_model_;

private:
	// Generate feedback central forces and torque based on model distance to the cloud
	std::pair<btVector3, btVector3>  generateDataForce(
		const btRigidBody &object, const std::string &model_name);
	
	std::pair<btVector3, btVector3> generateDataForceWithICP(PointCloudXYZPtr input_cloud,
		const btTransform &object_pose, const std::string &object_id);
	std::pair<btVector3, btVector3> generateDataForceWithSavedICP(PointCloudXYZPtr input_cloud,
		const btTransform &object_pose, const std::string &object_id);
	void updateCachedIcpResultMap(const PointCloudXYZPtr icp_result, const std::string &object_id);

	std::pair<btVector3, btVector3> calculateDataForceFromCorrespondence(
		const PointCloudXYZPtr input_cloud, const PointCloudXYZPtr target_cloud,
		const btVector3 &object_cog, const double &icp_confidence = 1.0) const;
	PointCloudXYZPtr doICP(const PointCloudXYZPtr input_cloud) const;
	std::pair<btVector3, btVector3> generateDataForceWithClosestPointPair(PointCloudXYZPtr input_cloud,
		const btTransform &object_pose) const;
	double getIcpConfidenceResult(const PointCloudXYZPtr icp_result, const double &voxel_size = 0.003) const;
	PointCloudXYZPtr generateCorrespondenceCloud(PointCloudXYZPtr input_cloud, 
		const bool &filter_distance = false, const double &max_distance = 0.0, const bool keep_index_aligned = true) const;
	PointCloudXYZPtr getTransformedObjectCloud(const btRigidBody &object, 
		const std::string &model_name) const;
	PointCloudXYZPtr getTransformedObjectCloud(const btRigidBody &object, 
		const std::string &model_name, btTransform &object_real_pose) const;
	PointCloudXYZPtr getTransformedObjectCloud(const btTransform &object_pose, 
		const std::string &model_name, btTransform &object_real_pose) const;

	std::string model_directory_;
	bool debug_;
	bool have_scene_data_;
	PointCloudXYZPtr scene_data_;
	pcl::KdTreeFLANN<pcl::PointXYZ> scene_data_tree_;
	std::map<std::string, PointCloudXYZPtr> model_cloud_map_;
	std::map<std::string, PointCloudXYZPtr> model_cloud_icp_result_map_;
	std::map<std::string, btScalar> icp_result_confidence_map_;
	std::map<std::string, btScalar> gravity_force_per_point_;
	std::map<std::string, btScalar> model_forces_scale_map_;
	btScalar percent_gravity_max_correction_;
	btScalar max_point_distance_threshold_;
	int max_icp_iteration_;
	pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ> icp_;
};

#endif