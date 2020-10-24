function err = error_determination(env, est_loc)

true_loc = env.sources(1,:);

true_loc_i = dot(true_loc, [1, 1i]);

est_loc_i = dot(est_loc, [1, 1i]);

dist_err = abs(true_loc_i) - abs(est_loc_i);
angle_err = wrapToPi(angle(true_loc_i) - angle(est_loc_i));

err = [dist_err, angle_err];





end

