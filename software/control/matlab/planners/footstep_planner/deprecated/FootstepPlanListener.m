classdef FootstepPlanListener
	properties
		lc
		aggregator
	end

	methods
		function obj = FootstepPlanListener(channel)
			obj.lc = lcm.lcm.LCM.getSingleton();
			obj.aggregator = lcm.lcm.MessageAggregator();
			obj.lc.subscribe(channel, obj.aggregator);
		end

		function [X, options] = getNextMessage(obj, t_ms)
			plan_msg = obj.aggregator.getNextMessage(t_ms);
			if isempty(plan_msg)
				X = []; options = struct();
			else
				[X, options] = FootstepPlanListener.decodeFootstepPlan(drc.deprecated_footstep_plan_t(plan_msg.data));
			end
		end

	end

	methods(Static)
		function [X, options] = decodeFootstepPlan(plan_msg)
		  for j = 1:length(plan_msg.footstep_goals)
		    X(j) = FootstepPlanListener.decodeFootstepGoal(plan_msg.footstep_goals(j));
		  end
		  options = struct('ignore_terrain', plan_msg.footstep_opts.ignore_terrain,...
		  	               'mu', plan_msg.footstep_opts.mu,...
		  	               'behavior', plan_msg.footstep_opts.behavior,...
		  	               'map_command', plan_msg.footstep_opts.map_command,...
		  	               'velocity_based_steps', plan_msg.footstep_opts.velocity_based_steps);
		end

		function X = decodeFootstepGoal(goal_msg)
		 rpy = quat2rpy([goal_msg.pos.rotation.w,...
		                        goal_msg.pos.rotation.x,...
		                        goal_msg.pos.rotation.y,...
		                        goal_msg.pos.rotation.z]);
		  X.pos = [goal_msg.pos.translation.x;...
		         goal_msg.pos.translation.y;...
		         goal_msg.pos.translation.z;...
		         rpy];

		  % Handle NaNs from network compression
		  if any(isnan(X.pos([1,2,6])))
		  	error('I don''t know how to handle NaN in x, y, or yaw');
		  else
		  	X.pos(isnan(X.pos)) = 0;
		  end

		  X.step_speed = goal_msg.step_speed;
		  X.step_height = goal_msg.step_height;
		  X.id = goal_msg.id;
		  X.pos_fixed = [goal_msg.fixed_x;
					           goal_msg.fixed_y;
					           goal_msg.fixed_z;
					           goal_msg.fixed_roll;
					           goal_msg.fixed_pitch;
					           goal_msg.fixed_yaw];
		  X.is_right_foot = goal_msg.is_right_foot;
		  X.is_in_contact = goal_msg.is_in_contact;
		  X.terrain_pts = [reshape(goal_msg.terrain_path_dist,1,[]); reshape(goal_msg.terrain_height,1,[])];
		end
	end
end