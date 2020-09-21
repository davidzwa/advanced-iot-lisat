function plot_result(env,y, u, t)

C = linspecer(env.num_mics + 1);


for r = 1:env.num_robots
    
    figure;
    plot(t,u,"Color",C(1,:),'DisplayName', "original signal");
    hold on
    
    for m = 1:env.num_mics
        plot(t,squeeze(y(r,m,:)), 'Color',C(1+m,:), 'DisplayName', "Mic: " + m);
    end
    
    legend
    title("received audio on robot " + r);
    xlabel("time[s]");
end


end

