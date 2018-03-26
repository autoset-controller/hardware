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
    max_speed = 5 * .15 * 2 * pi; % one rotation per second
    
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
            if i == wp_current    
                drawPosVec(waypoints(i,:), 0);
            else
                drawPosVec(waypoints(i,:), 1);
            end
        end
        drawPosVec(pos, 2);
        xlim([-6, 6]);
        ylim([-6, 6]);
        
        %Check if waypoint is passed, and update waypoint
        ispassed = check_passed(pos, wp_target);
        if(ispassed == 1)
            wp_current = wp_current + 1;
            if(wp_current > wp_count)
                wp_current = 1;
                complete_cycles = complete_cycles + 1;
            end
            wp_target = waypoints(wp_current, :);
        end
         
        %Find intersect and distances to intersect
        [intersect, radii] = get_intersect(pos, wp_target);
        
        %Determine appropriate ratio for current scenario
        ratio = get_ratio(speed, p_base_e, p_base_e, pos, wp_target, intersect, radii);
        
        %Set speed of motors: In final version, send out packet
        speed = ratio * max_speed;
        
        %Simulate movement of platform: Not used in final version
        enc = speed * timestep;
        pos = update_pos(pos, enc, p_radius, p_base_e, p_base_e);
        
        hold on
        
        %Advance timestep
        waitforbuttonpress
    end
    
end

function ratio = get_ratio(speed, p_base_m, p_base_e, pos, wp_target, intersect, radii)
    wp_r = radii(1);
    pos_r = radii(2);
    
    new_speed = speed;
    
    % determine whether waypoint is facing clockwise or counter-clockwise
    wp_intersect_theta = mod(atan2(intersect(2) - wp_target(2), intersect(1) - wp_target(1)), 2*pi); % angle towards intersect
    wp_theta = mod(wp_target(3), 2*pi);
    wp_dir = 1; %-1 = ccw, 1 = cw
    
    if(mod(wp_theta - wp_intersect_theta, 2*pi) >= pi)
        wp_dir = -1;
    end
    
    % determine whether platform is moving clockwise or counter-clockwise
    pos_intersect_theta = mod(atan2(intersect(2) - pos(2), intersect(1) - pos(1)), 2*pi); % angle towards interesect
    pos_theta = mod(pos(3), 2*pi);
    pos_dir = 1; %-1 = ccw, 1 = cw
    
    if(mod(pos_theta - pos_intersect_theta, 2*pi) >= pi)
        pos_dir = -1;
    end
    [wp_theta - wp_intersect_theta, pos_theta - pos_intersect_theta];
    [wp_dir, pos_dir];
    [wp_r, pos_r]
    if(wp_r < 30) 
        % get ratio for on-circle movement
        if(wp_dir == 1)
            default_speed = [2 * (wp_r + (p_base_m/2)) * pi, 2 * (wp_r - (p_base_m/2)) * pi];        
        else
            default_speed = [2 * (wp_r - (p_base_m/2)) * pi, 2 * (wp_r + (p_base_m/2)) * pi];
        end
        default_speed = default_speed / norm(default_speed);
        
        % determine change based on ratio of radii
        r_ratio = pos_r/wp_r
        r_diff = abs(pos_r - wp_r);
        
        % assign speeds based on relative radii
        % if platform is on the circle
        if(round(r_ratio, 3) == 1)
            new_speed = default_speed;
        % if platform is inside the circle
        elseif(r_ratio < 1)
            "inside";
            if(wp_dir == 1) % if waypoint is clockwise
                if(pos_dir == 1) % if moving clockwise
                    new_speed = [default_speed(1) - abs(default_speed(1)) * r_ratio, default_speed(2)];
                else % if moving counter-clockwise
                    new_speed = [default_speed(1), default_speed(2) - abs(default_speed(2)) * r_ratio];
                end
            else % if waypoint is counter-clockwise
                if(pos_dir == -1) % if moving counter-clockwise
                    new_speed = [default_speed(1), default_speed(2) - abs(default_speed(2)) * r_ratio];
                else % if moving clockwise
                    new_speed = [default_speed(1) - abs(default_speed(1)) * r_ratio, default_speed(2)];
                end
            end
        % if platform is outside the circle
        else
            "outside";
            if(wp_dir == 1) % if waypoint is clockwise
                if(pos_dir == 1) % if moving clockwise
                    new_speed = [default_speed(1), default_speed(2) - abs(default_speed(2)) * r_ratio];
                else % if moving counter-clockwise
                    new_speed = [default_speed(1) - abs(default_speed(1)) * r_ratio, default_speed(2)];
                end
            else % if waypoint is counter-clockwise
                if(pos_dir == -1) % if moving counter-clockwise
                    new_speed = [default_speed(1) - abs(default_speed(1)) * r_ratio, default_speed(2)];
                else % if moving clockwise
                    new_speed = [default_speed(1), default_speed(2) - abs(default_speed(2)) * r_ratio];
                end
            end
        end
        new_speed = new_speed / norm(new_speed);
    else
        % platform is oriented parallel to waypoint
        % TODO: Figure out how to do this
        "parallel"
        side = check_passed(pos, [wp_target(1), wp_target(2), wp_target(3) + pi/2]);
        
        if(side == 0) % platform is to the right of the waypoint
            if(pos_dir == wp_dir) % if platform is moving toward the waypoint
                new_speed = [0,1]; % turn to the left
            else % if platform is moving away from the waypoint
                new_speed = [1,0]; % turn to the right
            end
        else % platform is to the left of the waypoint
            if(pos_dir == wp_dir) % if platform is moving toward the waypoint
                new_speed = [1,0]; % turn to the right
            else % if platform is moving away from the waypoint
                new_speed = [0,1]; % turn to the left
            end
        end
    end
    
    ratio = new_speed
end

function passed = check_passed(pos, wp_target)
    passed = 0;
    
    dir = 0;

    %get linear equation for wp normal
    wp_m = round(round(cos(wp_target(3)), 10) / -round(sin(wp_target(3)), 10), 6);
    if(~isinf(wp_m))
        wp_b = -1 * (wp_target(1) * wp_m - wp_target(2));
        if(mod(wp_target(3), 2*pi) < pi)
            dir = 1; % point is passed when position y is greater than waypoint y
        else
            dir = -1; % point is passed when position y is less than waypoint y
        end
    else
        wp_b = wp_target(1);
        if(mod(wp_target(3), 2*pi) > pi/2 && mod(wp_target(3), 2*pi) < 3*pi/2)
            dir = 1; % point is passed when position x is less than waypoint x
        else
            dir = -1; % point is passed when position x is greater than waypoint x
        end
    end
   
    norm_y = wp_m * pos(1) + wp_b;
    if(~isinf(wp_m))
        if(dir * pos(2) > dir * norm_y)
            passed = 1;
        end
    else
        if(dir * pos(1) < dir * wp_b)
            passed = 1;
        end
    end
end

function [intersect, radii] = get_intersect(pos, wp_target)

    %CALCULATE CENTER
    %get linear equation for pos normal
    %process in form mx - y = -b
    pos_m = round(cos(pos(3)) / -sin(pos(3)), 6);
    pos_b = -1 * (pos(1) * pos_m - pos(2));
    center_x = NaN;
    if(isinf(pos_m))
        center_x = pos(1);
        pos_b = pos(1);
    end
    %get linear equation for wp normal
    wp_m = round(cos(wp_target(3)) / -sin(wp_target(3)), 6);
    wp_b = -1 * (wp_target(1) * wp_m - wp_target(2));
    
    if(isinf(wp_m))
        center_x = wp_target(1);
        wp_b = wp_target(1);
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
            if(isinf(pos_m))
                center_y = wp_m*center_x + wp_b;
            else
                center_y = pos_m*center_x + pos_b;
            end
            intersect = [center_x, center_y];
        %standard case
        else
            m = [pos_m, -1; wp_m, -1];
            m = inv(m);
            intersect = m * [pos_b;wp_b];
            intersect = -intersect';
            
            plot([-5, 5], [pos_m*-5 + pos_b, pos_m*5 + pos_b], 'r-')
            plot([-5, 5], [wp_m*-5 + wp_b, wp_m*5 + wp_b], 'b-')
            plot(intersect(1), intersect(2), 'o', 'Color', [.8, 0, .8])
        end
    end
    
    if(isnan(intersect(1)))
        % special case: normals are parallel
        wp_r = NaN;
        pos_r = NaN;
    else
        % find distances to center
        wp_r =  sqrt((intersect(1) - wp_target(1))^2 + (intersect(2) - wp_target(2))^2);
        pos_r = sqrt((intersect(1) - pos(1))^2 + (intersect(2) - pos(2))^2);
       
        % plot circles
        rectangle('Position',[intersect(1) - pos_r, intersect(2) - pos_r, pos_r * 2, pos_r * 2],'Curvature',[1,1]);
        rectangle('Position',[intersect(1) - wp_r, intersect(2) - wp_r, wp_r * 2, wp_r * 2],'Curvature',[1,1]);
        % determine case for position values
        if(round(pos_r, 3) == 0)
            % 
        end
    end
    
    radii = [wp_r, pos_r];
end

function new_pos = update_pos(pos, enc, p_radius, p_base_m, p_base_e)
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
        ls_line = 'g-';
        ls_point = 'go';
    elseif linespec == 1
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