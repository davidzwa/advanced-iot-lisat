function plot_est(sources_per_ring, ring_distances, est_loc,env)

figure
hold on

for i = 1:length(ring_distances)
    
    
    s = quiver(zeros(sources_per_ring,1),zeros(sources_per_ring,1), ...
        ring_distances(i)*est_loc(1+(i-1)*sources_per_ring:i*sources_per_ring,1), ...
        ring_distances(i)*est_loc(1+(i-1)*sources_per_ring:i*sources_per_ring,2),0,'k');
    
    s.Annotation.LegendInformation.IconDisplayStyle = 'off';
    
end


C = linspecer(env.num_robots);


%sound sources
%scatter(env.sources(:,1), env.sources(:,2),'k^','filled','DisplayName', 'Speakers');

for i = 1:env.num_robots
    %robot location
    %scatter(env.robots(i,1),env.robots(i,2),'filled','MarkerFaceColor',C(i,:));
    quiver(env.robots(i,1),env.robots(i,2),cos(env.robots(i,3)),sin(env.robots(i,3)),0.5,...
        "Color",C(i,:),"MaxHeadSize",0.7, 'DisplayName', "robot: " + i);
    
    %mic locations
    s = scatter(env.mics(:,1,i),env.mics(:,2,i),'filled','o','MarkerEdgeColor',C(i,:));
    s.Annotation.LegendInformation.IconDisplayStyle = 'off';
end

legend

xlabel("x[m]");
ylabel("y[m]");

    camroll(90)


end

