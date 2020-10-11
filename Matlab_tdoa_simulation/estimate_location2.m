function est_location = estimate_location2(env, c, tdoa)

disp take_2

%source: https://d1wqtxts1xzle7.cloudfront.net/3387073/20100917.pdf?response-content-disposition=inline%3B+filename%3DAn_Improved_Chan_Ho_Location_Algorithm_f.pdf&Expires=1600940148&Signature=DeVSkQWjUAQBTjkp1Zmh0x3h974ZNdl7MrVuS8Us9kugwND9tLn6auOw1nkROK-PnTiJY8Nai570yEGEbTZaujxXI4E9-ZVzEW0XorMkfNdUBjKLhdnZB-jl2MpwTyXKCWvPnmPScuUBmOfBYhoviGp3eZSbb54yPXit7OWy99U4Dq0CPnur7wjnpM0D5wUINBtXs5RToHdG-MaDP4I9DY61J8UdNTlx7X2nTMN2eYIzKxTa-KiqGieBYFVtxW9NAFsAzVNpYcY92sfMe0yFundw5wTPXMolJHS1O5agsg0GNiqNEJ9hUOZGbjTBj5tRG5mHnbrv-ZgGifESzbBoWQ__&Key-Pair-Id=APKAJLOHF5GGSLRBV4ZA
% Chan-Ho model

est_location = zeros(env.num_robots, 2);
max_it = 10;
a = zeros(max_it,2);

%  env.mics_relative(2:end,2)
for Rob = 1:env.num_robots
    
    K = env.mics_relative(:,1).^2 + env.mics_relative(:,2).^2;
    r = c * abs(tdoa(Rob,:)');  % range differences
    
    X = env.mics_relative(2:end, 1) - env.mics_relative(1,1);   % differences in x coordinates w.r.t. x1
    Y = env.mics_relative(2:end, 2) - env.mics_relative(1,2);   % differences in y coordinates w.r.t. y1
    
    x = zeros(env.num_mics-1,1);

    
    for i = 1:max_it
    
        R = sqrt( K(1) - 2*env.mics_relative(1,1)*x(1) ...
                    - 2*env.mics_relative(1,2)*x(2) + x(1)^2 + x(2)^2);
                
        x = -inv([X(1), Y(1); X(2), Y(2)]) *([r(1);r(2)]*R + 0.5*[r(:).^2 - K(2:end) + K(1)]);
        a(i,:) = x(1:2)';

    end
    
    est_location(Rob,:) = x(1:2)';
    scatter(a(:,1), a(:,2));scatter(a(end,1), a(end,2),'+');

end



end