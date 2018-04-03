/**
 * Checks which side the platform is on in relation to a waypoint
 *
 * @param {Object} platformPos Position and orientation of platform
 * @param {Object} waypointPos Position and orientation of waypoint
 * @returns {Number} 1 = passed, 0 = on line, -1 = behind the line
 */
function checkSide(platformPos, waypointPos) {
  let passed = 0;
  let dir = 0;
  let bConstant;

  // get linear equation for wp normal
  const mSlope = parseFloat(parseFloat(Math.cos(waypointPos.theta).toFixed(10)) /
  -parseFloat(Math.sin(waypointPos.theta).toFixed(10)).toFixed(6));
  if (!Number.isFinite(mSlope)) {
    bConstant = -1 * (waypointPos.x * mSlope - waypointPos.y);
    if ((waypointPos.theta % 2 * Math.PI) < Math.PI)
      dir = 1; // point is passed when position y is greater than waypoint y
    else
      dir = -1; // point is passed when position y is less than waypoint y
  } else {
    bConstant = waypointPos.x;
    if ((waypointPos.theta % 2 * Math.PI) > Math.PI / 2 &&
    (waypointPos.theta % 2 * Math.PI) < 3 * Math.PI / 2)
      dir = 1; // point is passed when position x is less than waypoint x
    else
      dir = -1; // point is passed when position x is greater than waypoint x
  }

  const normalY = mSlope * platformPos.x + bConstant;
  if (!Number.isFinite(mSlope)) {
    if (dir * platformPos.y > dir * normalY)
      passed = 1;
    else if (parseFloat(dir * platformPos.y).toFixed(2) === dir * normalY)
      passed = 0;
    else
      passed = -1;
  } else if (dir * platformPos.x < dir * bConstant)
    passed = 1;
  else if (parseFloat(dir * platformPos.x).toFixed(2) === dir * bConstant)
    passed = 0;
  else
    passed = -1;

  return passed;
}

/**
 * Returns the new speed of the platform on its destination to a waypoint
 *
 * @param {Object} speed Speed of left and right motors
 * @param {Number} motorDistance Distance between motors (.6)
 * @param {any} platformPos (x,y,theta) of platform
 * @param {any} waypointPos (x,y,theta) of destination
 * @param {Object} intersect (x,y) of point of intersection
 * @param {Object} radii Radius of waypoint and platform
 * @returns {Object} newSpeed Normarlized speed ratio
 */
function getRatio(
  speed, motorDistance, platformPos,
  waypointPos, intersect, radii
) {
  const waypointRadius = radii.waypoint;
  const platformRadius = radii.platform;
  let defaultSpeed;
  let newSpeed = speed;

  // determine whether waypoint is facing clockwise or counter-clockwise
  const waypointIntersectAngle = Math.atan2(
    intersect.y - waypointPos.y,
    intersect.x - waypointPos.x
  ) % 2 * Math.PI; // angle towards intersect
  const waypointAngle = waypointPos.theta % 2 * Math.PI;
  let waypointDirection = 1; // -1 = ccw, 1 = cw

  if (waypointAngle - waypointIntersectAngle % 2 * Math.PI >= Math.PI)
    waypointDirection = -1;

  // determine whether platform is moving clockwise or counter-clockwise
  const platformIntersectAngle = Math.atan2(
    intersect.y - platformPos.y,
    intersect.x - platformPos.x
  ) % 2 * Math.PI; // angle towards interesect
  const platformAngle = platformPos.theta % 2 * Math.PI;
  let platformDirection = 1; // -1 = ccw, 1 = cw

  if (((platformAngle - platformIntersectAngle) % 2 * Math.PI) >= Math.PI)
    platformDirection = -1;
  if (waypointRadius < 30) {
    // get ratio for on-circle movement
    if (waypointDirection === 1) {
      if (platformDirection === 1)
        defaultSpeed = [2 * (waypointRadius + (motorDistance / 2)) * Math.PI,
          2 * (waypointRadius - (motorDistance / 2)) * Math.PI];
      else
        defaultSpeed = [2 * (waypointRadius + (motorDistance / 2)) * Math.PI,
          2 * (waypointRadius - (motorDistance / 2)) * Math.PI];
    } else if (platformDirection === -1)
      defaultSpeed = [2 * (waypointRadius - (motorDistance / 2)) * Math.PI,
        2 * (waypointRadius + (motorDistance / 2)) * Math.PI];
    else
      defaultSpeed = [2 * (waypointRadius - (motorDistance / 2)) * Math.PI,
        2 * (waypointRadius + (motorDistance / 2)) * Math.PI];

    defaultSpeed = {
      left: defaultSpeed.left / Math.sqrt(defaultSpeed.left ** 2 + defaultSpeed.right ** 2),
      right: defaultSpeed.right / Math.sqrt(defaultSpeed.left ** 2 + defaultSpeed.right ** 2)
    };

    // determine change based on ratio of radii
    const radiusRatio = platformRadius / waypointRadius;

    // assign speeds based on relative radii
    // if platform is on the circle
    if (parseFloat(radiusRatio.toFixed(3)) === 1)
      newSpeed = defaultSpeed;
    // if platform is inside the circle
    else if (radiusRatio < 1) {
      if (waypointDirection === 1) { // if waypoint is clockwise
        if (platformDirection === 1) // if moving clockwise
          newSpeed = [defaultSpeed.left - Math.abs(defaultSpeed.left) *
            radiusRatio, defaultSpeed.right];
        else // if moving counter-clockwise
          newSpeed = [defaultSpeed.left, defaultSpeed.right - Math.abs(defaultSpeed.right) *
            radiusRatio];
      } else if (platformDirection === -1) // if moving counter-clockwise
        newSpeed = [defaultSpeed.left, defaultSpeed.right - Math.abs(defaultSpeed.right) *
          radiusRatio];
      else // if moving clockwise
        newSpeed = [defaultSpeed.left - Math.abs(defaultSpeed.left) *
          radiusRatio, defaultSpeed.right];
      // if platform is outside the circle
    } else if (waypointDirection === 1) { // if waypoint is clockwise
      if (platformDirection === 1) // if moving clockwise
        newSpeed = [defaultSpeed.left, defaultSpeed.right - Math.abs(defaultSpeed.right) *
          radiusRatio];
      else // if moving counter-clockwise
        newSpeed = [defaultSpeed.left - Math.abs(defaultSpeed.left) *
          radiusRatio, defaultSpeed.right];
    } else if (platformDirection === -1) // if moving counter-clockwise
      newSpeed = [defaultSpeed.left - Math.abs(defaultSpeed.left) *
        radiusRatio, defaultSpeed.right];
    else // if moving clockwise
      newSpeed = [defaultSpeed.left, defaultSpeed.right - Math.abs(defaultSpeed.right) *
        radiusRatio];

    newSpeed = {
      left: newSpeed.left / Math.sqrt(newSpeed.left ** 2 + newSpeed.right ** 2),
      right: newSpeed.right / Math.sqrt(newSpeed.left ** 2 + newSpeed.right ** 2)
    };
  } else {
    // platform is oriented parallel to waypoint
    const side = checkSide(
      platformPos,
      [waypointPos.x, waypointPos.y, waypointPos.theta + Math.PI / 2]
    );

    if (side === -1) { // platform is to the right of the waypoint
      if (platformDirection === waypointDirection) // if platform is moving toward the waypoint
        newSpeed = [0, 1]; // turn to the left
      else // if platform is moving away from the waypoint
        newSpeed = [1, 0]; // turn to the right
    } else if (side === 1) { // platform is to the left of the waypoint
      if (platformDirection === waypointDirection) // if platform is moving toward the waypoint
        newSpeed = [1, 0]; // turn to the right
      else // if platform is moving away from the waypoint
        newSpeed = [0, 1]; // turn to the left
    } else // platform is in line with the waypoint
      newSpeed = [1, 1];
  }


  newSpeed = {
    left: newSpeed.left / Math.sqrt(newSpeed.left ** 2 + newSpeed.right ** 2),
    right: newSpeed.right / Math.sqrt(newSpeed.left ** 2 + newSpeed.right ** 2)
  };

  return newSpeed;
}

/**
 * Calculates the intersection between position and waypoint
 * @param {Object} platform Platform's position (x, y, theta)
 * @param {Object} waypoint Waypoint's position (x, y, theta)
 * @return {Object} The point of intersection between the platform
 * and waypoint and the radius
 */
function getIntersect(platform, waypoint) {
  // CALCULATE CENTER
  // get linear equation for pos normal
  // process in form mx - y = -b
  const mPlatform = parseFloat(Math.cos(platform.theta) / -Math.sin(platform.theta).toFixed(6));
  let bPlatform = -1 * (platform.x * mPlatform - platform.y);
  let xCenter = NaN;
  let yCenter = NaN;
  let waypointRadius;
  let platformRadius;
  let intersect = { x: 0, y: 0 };

  if (!Number.isFinite(mPlatform)) {
    intersect.x = platform.x;
    bPlatform = platform.x;
  }
  // get linear equation for wp normal
  const mWaypoint = parseFloat(Math.cos(waypoint.theta) / -Math.sin(waypoint.theta).toFixed(6));
  let bWaypoint = -1 * (waypoint.x * mWaypoint - waypoint.y);

  if (!Number.isFinite(mWaypoint)) {
    xCenter = waypoint.x;
    bWaypoint = waypoint.x;
  }

  // if normals are parallel, handle special case
  if (mPlatform === mWaypoint) {
    // special case: normals are parallel
    xCenter = NaN;
    intersect = [xCenter, yCenter];
    // find intersect
    // if one normal is vertical
  } else if (!Number.isNaN(xCenter)) {
    if (!Number.isFinite(mPlatform))
      yCenter = mWaypoint * xCenter + bWaypoint;
    else
      yCenter = mPlatform * xCenter + bPlatform;
    intersect = [xCenter, yCenter];
    // standard case
  } else {
    intersect.x = (bWaypoint - bPlatform) / (mWaypoint - mPlatform);
    intersect.y = intersect.x * mPlatform + bPlatform;
  }

  if (Number.isNaN(intersect.x)) {
    // special case: normals are parallel
    waypointRadius = NaN;
    platformRadius = NaN;
  } else {
    // find distances to center
    waypointRadius = Math.sqrt((intersect.x - waypoint.x) ** 2 + (intersect.y - waypoint.y) ** 2);
    platformRadius = Math.sqrt((intersect.x - platform.x) ** 2 + (intersect.y - platform.y) ** 2);
    // determine case for position values
    // if(round(pos_r, 3) == 0)
  }

  const radii = [waypointRadius, platformRadius];
  return { intersect, radii };
}

export {
  checkSide, getRatio, getIntersect
};
