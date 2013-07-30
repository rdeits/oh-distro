classdef cubicSplineTraj
	properties
		X
		dX
		lambda
	end

	methods
		function obj = cubicSplineTraj(poses)
      sizecheck(poses, [6,2]);
      start_pos = poses(:,1);
      goal_pos = poses(:,2);
			p0 = start_pos(1:2);
			pf = goal_pos(1:2) - p0;
			initial_angle = start_pos(6);
			R = [cos(-initial_angle), -sin(-initial_angle); ...
			  sin(-initial_angle), cos(-initial_angle)];
			pf = R * pf;
			final_angle = goal_pos(6) - initial_angle;

			dydx0 = 0;
			dydxf = tan(final_angle);

			pp = pchipDeriv([0, pf(1)], [0, pf(2)], [dydx0, dydxf]);

			x = linspace(0, pf(1), 100);
			y = ppval(pp, x);
			dydx = diff(y) ./ diff(x);

			X = [x', y'];
			dX = [diff(x'), diff(y')];
			dX = [dX; dX(end,:)];
			lambda = cumsum(sqrt([0, diff(x)].^2 + [0, diff(y)].^2));

			obj.X = X * inv(R)' + repmat(p0', length(X(:,1)), 1);
			obj.dX = dX * inv(R)';
			obj.lambda = lambda;
		end

		function Xi = eval(obj, li)
      li = max(zeros(size(li)), li);
      li = min(ones(size(li)), li);
			li = li * obj.lambda(end);
			trans = interp1(obj.lambda, obj.X, li);
			dXi = interp1(obj.lambda, obj.dX, li);
			dXi = dXi ./ repmat(sqrt(sum(dXi .^ 2, 2)), 1, 2);
			yaw = atan2(dXi(:,2), dXi(:,1));
			Xi = [trans'; zeros(3, length(li)); yaw'];
		end
	end
end