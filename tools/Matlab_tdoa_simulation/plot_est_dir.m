function plot_est_dir(env, est_dir, fig)

C = linspecer(env.num_robots);

figure(fig)
hold on

U = env.robots(:,1:2)-env.sources(1,1:2);
U = -U./sqrt(sum(U.^2,2));
quiver(env.robots(:,1),env.robots(:,2),U(:,1),U(:,2),0,'k', 'DisplayName',"true direction");

for Rob = 1:env.num_robots

    quiver(env.robots(Rob,1),env.robots(Rob,2),est_dir(Rob,1),est_dir(Rob,2),0,...
        "Color",C(Rob,:),'DisplayName',"Est robot: " + Rob);
end


end

