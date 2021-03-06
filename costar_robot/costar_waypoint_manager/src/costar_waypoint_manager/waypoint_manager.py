'''
By Chris Paxton
(c) 2016-2017 The Johns Hopkins University
'''

import os
import rospy
import tf
import tf_conversions.posemath as pm
import yaml

from predicator_msgs.srv import *
from predicator_msgs.msg import *

from costar_component import CostarComponent
from costar_robot_msgs.srv import *

from librarian_msgs.srv import *
from librarian_msgs.msg import *

'''
Set up a basic waypoint manager. This object will store a list of joint space
or cartesian waypoints (the later relative to some frame of reference).

These are stored in Librarian and get loaded any time the system comes up.
'''
class WaypointManager(CostarComponent):

  def __init__(self,
          world="world",
          namespace="",
          endpoint="/endpoint",
          service=False,
          broadcaster=None):

    super(WaypointManager, self).__init__(name="WaypointManager", namespace=namespace)

    self.namespace = namespace

    if not broadcaster is None:
        self.broadcaster = broadcaster
    else:
        self.broadcaster = tf.TransformBroadcaster()

    # Require librarian services.
    rospy.wait_for_service('/librarian/add_type',5)
    rospy.wait_for_service('/librarian/load',5)
    self.add_type_service = rospy.ServiceProxy('/librarian/add_type', librarian_msgs.srv.AddType)
    self.save_service = rospy.ServiceProxy('/librarian/save', librarian_msgs.srv.Save)
    self.load_service = rospy.ServiceProxy('/librarian/load', librarian_msgs.srv.Load)
    self.list_service = rospy.ServiceProxy('/librarian/list', librarian_msgs.srv.List)
    self.delete_service = rospy.ServiceProxy('/librarian/delete', librarian_msgs.srv.Delete)

    self.world = world
    self.endpoint = endpoint

    self.js_folder = 'joint_states'
    self.cart_folder = 'instructor_waypoint'

    self.js_waypoints = {}
    self.cart_waypoints = {}
    self.all_js_moves = []
    self.all_cart_moves = []

    # If we are bringing this up as an independent component...
    # This will create the appropriate services
    if service:
      self.save_frame = self.make_service('SaveFrame',SaveFrame,self.save_frame_cb)
      self.save_joints = self.make_service('SaveJointPosition',SaveFrame,self.save_joints_cb)
      self.get_joint_states_waypoints_list = self.make_service('GetJointStateWaypoints',GetList,self.get_js_waypoints_list_cb)
      self.get_waypoints_list = self.make_service('GetWaypointsList',GetList,self.get_waypoints_list_cb)

    self.update()

  '''
  Save the current frame.
  '''
  def save_frame_cb(self, req):
      raise NotImplementedError('Function not yet implemented!')
      pass

  '''
  Save the current set of joint positions.
  '''
  def save_joints_cb(self, req):
      raise NotImplementedError('Function not yet implemented!')

  '''
  Return the list of joint state waypoints
  '''
  def get_js_waypoints_list_cb(self,req):
    self.update()
    return GetListResponse(items=self.js_waypoints.keys())

  '''
  Return the list of cartesian waypoints
  '''
  def get_waypoints_list_cb(self,req):
    self.update()
    return GetListResponse(items=self.cart_waypoints.keys())

  '''
  Update list of frames from librarian.
  These are all saved on disk somewhere or another.
  '''
  def update(self):
    
    # ----------------------------------------
    # this section loads joint space waypoints
    js_filenames = self.list_service(self.js_folder).entries

    self.js_waypoints = {}
    self.all_js_moves = []
    for name in js_filenames:
      data = yaml.load(self.load_service(id=name,type=self.js_folder).text)
      self.js_waypoints[name] = data
      self.all_js_moves.append(name)

    # ----------------------------------------
    # this section loads cartesian waypoints
    cart_filenames = self.list_service(self.cart_folder).entries
    self.cart_waypoints = {}
    self.all_cart_moves = []
    for name in cart_filenames:
      data = yaml.load(self.load_service(id=name,type=self.cart_folder).text)
      self.cart_waypoints[name] = data
      self.all_cart_moves.append(name)

  '''
  Save frame to library if necessary
  '''
  def save_frame(self, frame, reference, name):
    pass

  '''
  Save joints to library if necessary
  '''
  def save_joints(self, joints, name):
      self.save_service(id=name.strip('/'),type=self.js_folder,text=yaml.dump(joints))

  '''
  Find a saved frame and return it
  '''
  def lookup_frame(self, name, reference):
    pass

  '''
  Display TF positions of all possible waypoints.
  '''
  def publish_tf(self):
    pass

  '''
  implement service to get joint states
  '''
  def get_joint_states_by_name_srv(self, req):
      if req.name in self.js_waypoints:
          msg = LookupJointStatesResponse(joint_states=self.js_waypoints[name], ack='SUCCESS')
      else:
          msg = LookupJointStatesResponse(ack='FAILURE - %s not found'%req.name)
      return msg
