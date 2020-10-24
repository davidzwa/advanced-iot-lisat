function plot_estimate(env, est_loc, fig)

C = linspecer(env.num_robots);

figure(fig)
hold on
for r = 1:env.num_robots
    scatter(est_loc(r,1), est_loc(r,2),45, 'MarkerEdgeColor',C(r,:),...
        'DisplayName',"Est robot: " + r);
end
end

