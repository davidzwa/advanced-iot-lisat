function plot_err(sources_per_ring, ring_distances, err,env)

%% ploting angle errors
% figure
% hold on
% 
% axis([env.field(1)-1, env.field(2)+1, env.field(3)-1, env.field(4)+1]);
% 
% pointsize = 10;
% scatter(env.sources(:,1), env.sources(:,2),pointsize,err(:,2),'filled','DisplayName', 'Speakers');
% colorbar
% caxis([quantile(err(:,2),0.01),quantile(err(:,2),0.99)])
% 
% quiver(env.robots(1,1),env.robots(1,2),cos(env.robots(1,3)),sin(env.robots(1,3)),0.5,...
%     'ok','DisplayName', "robot","MaxHeadSize",0.7);
% 
% %mic locations
% s = scatter(env.mics(:,1,1),env.mics(:,2,1),'+k');
% s.Annotation.LegendInformation.IconDisplayStyle = 'off';

figure

angle = linspace(0, 360, sources_per_ring);

Z = zeros(sources_per_ring, length(ring_distances));

for i = 1:length(ring_distances)
   Z(:,i) = err((i-1)*sources_per_ring + 1:i*sources_per_ring,2);
    
end

Z = flip(Z,1);
min_err = min(err(:,2));
max_err = max(err(:,2));

pos = [0.11, 0.3, 0.5, 0.7, 0.9, 1]

[h, c] = polarPcolor(ring_distances, angle, rad2deg(Z),'labelR','r [m]','circlesPos',pos, 'Nspokes', 9);
ylabel(c,'Angular error [degrees]');
c.FontSize = 16;
h.Annotation.LegendInformation.IconDisplayStyle = 'off';
% title("Angular error");

l = findobj('Type', 'Line');
for i = 1:length(l)
   l(i).Annotation.LegendInformation.IconDisplayStyle = 'off';
end



hold on

quiver(0,0,cos(env.robots(1,3)+0.5*pi),sin(env.robots(1,3)+0.5*pi),0.1,...
         'ok','DisplayName', "robot","MaxHeadSize",0.7);
s = scatter(-env.mics(:,2,1),env.mics(:,1,1),'+k',"DisplayName", "Microphone");
% s.Annotation.LegendInformation.IconDisplayStyle = 'off';

legend
%caxis([min(Z(:)),max(Z(:))])



end

