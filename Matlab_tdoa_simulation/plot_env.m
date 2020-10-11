function fig_nr = plot_env(env)

fig_nr = figure();

if env.walls
    line = '-';
else
    line = '--';
end

%rectangle('Position',[env.field], 'LineStyle', line);
axis([env.field(1)-1, env.field(2)+1, env.field(3)-1, env.field(4)+1]);
%set(gca,'XColor', 'none','YColor','none')

C = linspecer(env.num_robots);


hold on


%sound sources
scatter(env.sources(:,1), env.sources(:,2),'k^','filled','DisplayName', 'Speakers');


for i = 1:env.num_robots
    %robot location
    %scatter(env.robots(i,1),env.robots(i,2),'filled','MarkerFaceColor',C(i,:));
    quiver(env.robots(i,1),env.robots(i,2),cos(env.robots(i,3)),sin(env.robots(i,3)),0.5,...
        "Color",C(i,:),"MaxHeadSize",0.7, 'DisplayName', "robot: " + i);
    
    %mic locations
    s = scatter(env.mics(:,1,i),env.mics(:,2,i),'+','MarkerEdgeColor',C(i,:));
    s.Annotation.LegendInformation.IconDisplayStyle = 'off';
end

legend

xlabel("x[m]");
ylabel("y[m]");

end

