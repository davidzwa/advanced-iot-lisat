clearvars
%close all
%quick test to see if cutting the plane method for direction determination
%could work


N_mic = 10;
mic_locs = [0.05,0; 0,0.1; 0,-0.1] + [0.075,0];
%mic_locs = 0.1*[cos(2*pi/N_mic*(0:N_mic-1)'), sin(2*pi/N_mic*(0:N_mic-1)')];


%% compute unit distance vectors
vect = zeros(nchoosek(length(mic_locs),2),2);
k = 1;
for i = 1:length(mic_locs)
    for j = i+1:length(mic_locs)
        temp = mic_locs(j,:) - mic_locs(i,:);
        vect(k,:) = temp/norm(temp);
        k = k+1;
    end
end


%% source

source = [2,-0.5];

%% TOA from mic to source 

TOA = zeros(length(mic_locs),1);
c = 343;    %m/s

for i = 1:length(mic_locs)
    TOA(i) = norm(source - mic_locs(i,:))/c;
    
end

%% direction estimation

ang = [0,0];

k = 1;
for i = 1:length(mic_locs)
    for j = i+1:length(mic_locs)
        
        if (TOA(i) > TOA(j))
            ang = ang + vect(k,:);
        else
            ang = ang - vect(k,:);
        end
        k = k+1;
    end
end

ang = ang/norm(ang)



%% plotting
figure

hold on
scatter(source(1), source(2),'k^','filled');
grid


scatter(mic_locs(:,1),mic_locs(:,2),'ro', 'filled');
k = 1;
for i = 1:length(mic_locs)
    for j = i+1:length(mic_locs)
        cent = (mic_locs(i,:)+mic_locs(j,:))./2;
        slope = (mic_locs(j,2)-mic_locs(i,2))/(mic_locs(j,1)-mic_locs(i,1));
        perp = tan( atan(slope)+pi/2); 
        x = [-3,3];
        if perp > 1e10
            x = [0,0];
            y = [-3,3];
        else
            y = (perp*(x-cent(1))+cent(2));
        end
        
        plot(x,y,'b')
        k = k+1';
        
    end
end


quiver(0,0,ang(1), ang(2),0.1,'k');


axis([-3, 3, -3, 3]);

xlabel("x[m]")
ylabel("y[m]")
title("Cutting the plane sections")
axis([-0.5, 0.5, -0.5, 0.5])
