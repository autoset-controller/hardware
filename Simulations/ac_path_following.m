function [] = ac_path_following(waypoints, initial, cycles)
    % Simulation for Autoset Controller path following
    % Inputs:
    % waypoints - vector of 1x3 position/orientation vectors
    %           - structure: ((x1, y1, t1); (x2, y2, t2); ...)
    % initial   - 1x3 position/orientation vector
    %           - structure: (x0, y0, t0)
    % cycles    - number of cycles to run before execution finishes
    %
    % Behavior:
    % Upon startup, program performs a timestep each time
    % the spacebar is pressed. Position, waypoints, and relevant
    % metrics are displayed.
    % 
    % Notes:
    % - All measurements are in meters
    
    % CONSTANTS:
    p_base_e = 1.5; %distance between encoder wheels
    p_base_m = 2.5; %distance between motor wheels
    p_radius = .15; %radius of encoder wheels
    max_speed = .15 * 2 * pi; % one rotation per second
    
    speed = [0, max_speed]; %left/right speeds in m/s
    timestep = .1;
    
    wp_count = size(waypoints, 1);
    pos = initial;
    
    wp_current = 1;
    wp_target = waypoints(wp_current, :);
    
    f = figure;
    xlim([-6, 6]);
    ylim([-6, 6]);
    
    % Draw Waypoints
    for i = 1:wp_count
        drawPosVec(waypoints(i,:), 0);
    end
        
    complete_cycles = 0;
    
    % MAIN LOOP
    target_speed = .5;
    while(complete_cycles < cycles)
        %render positions of platform and waypoint
        hold off
        % Draw Waypoints
        for i = 1:wp_count
            drawPosVec(waypoints(i,:), 0);
        end
        drawPosVec(pos, 1);
        xlim([-6, 6]);
        ylim([-6, 6]);
        ratio = get_ratio(pos, wp_target);
        enc = speed * timestep;
        pos = update_pos(pos, enc, p_radius, p_base_e);
        hold on
        waitforbuttonpress
        %set_speed(ratio, target_speed);
    end
    
end

function ratio = get_ratio(pos, wp_target)

    %CALCULATE CENTER
    %get linear equation for pos normal
    %process in form mx - y = -b
    pos_m = round(cos(pos(3)) / -sin(pos(3)), 6);
    pos_b = -1 * (pos(1) * pos_m - pos(2));
    center_x = NaN;
    if(pos_m == Inf)
        center_x = pos(1);
    end
    %get linear equation for wp normal
    wp_m = round(cos(wp_target(3)) / -sin(wp_target(3)), 6);
    wp_b = -1 * (wp_target(1) * wp_m - wp_target(2));
    
    if(wp_m == Inf)
        center_x = pos(1);
    end
    
    %if normals are parallel, handle special case
    if(pos_m == wp_m)
        % special case: normals are parallel
        center_x = NaN;
        center_y = NaN;
    else
        % find intersect
        % if one normal is vertical
        if(~isnan(center_x))
            if(isnan(pos_b))
                center_y = wp_m*center_x + wp_b;
            else
                center_y = pos_m*center_x + pos_b;
            end
            center = [center_x, center_y];
        %standard case
        else
            m = [pos_m, -1; wp_m, -1];
            m = inv(m);
            center = m * [pos_b;wp_b];
            center = -center';
            
            plot([-5, 5], [pos_m*-5 + pos_b, pos_m*5 + pos_b], 'r-')
            plot([-5, 5], [wp_m*-5 + wp_b, wp_m*5 + wp_b], 'b-')
            plot(center(1), center(2), 'o', 'Color', [.8, 0, .8])
        end
    end
    
    if(isnan(center(1)))
        % special case: normals are parallel
        pos_d2c = NaN;
        wp_d2c = NaN;
    else
        % find distances to center
        pos_d2c = sqrt((center(1) - pos(1))^2 + (center(2) - pos(2))^2)
        wp_d2c =  sqrt((center(1) - wp_target(1))^2 + (center(2) - wp_target(2))^2)
        
        % determine case for position values
        if(round(pos_d2c, 3) == 0)
            % 
        end
    end
    
    ratio = [0,.2];
end

function new_pos = update_pos(pos, enc, p_radius, p_base_e)
    dL = enc(1);
    dR = enc(2);
    dt = p_radius / (2 * p_base_e) * (dR - dL);
    
    t_new = pos(3) + dt;
    dp = p_radius/2 * [cos(t_new), cos(t_new); sin(t_new), sin(t_new)] * [dR; dL];
    p_vec = [pos(1); pos(2)] + [dp(1); dp(2)];
    x_new = p_vec(1);
    y_new = p_vec(2);
    
    new_pos = [x_new, y_new, t_new];
end

function drawPosVec(pos, linespec)
    %get display parameters
    if linespec == 0
        ls_line = 'b-';
        ls_point = 'bo';
    else
        ls_line = 'r-';
        ls_point = 'ro';
    end
    x1 = pos(1) + 0.5 * cos(pos(3));
    y1 = pos(2) + 0.5 * sin(pos(3));
    %plot point
    plot(pos(1), pos(2), ls_point);
    hold on
    %plot arrow
    plot([pos(1), x1], [pos(2), y1], ls_line);
end