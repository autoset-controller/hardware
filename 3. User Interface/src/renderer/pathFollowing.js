function mod(n, m) {
  return ((n % m) + m) % m;
}

/**
 * Calculates the intersection between position and waypoint
 * @param {Object} platform Platform's position (x, y, theta)
 * @param {Object} waypoint Waypoint's position (x, y, theta)
 * @return {Object} The point of intersection between the platform
 * and waypoint and the radius
 */
function getIntersect(platform, waypoint) {
  // DEFINE VECTORS

  // for p + tq == r + us, calculate the following to solve for u:
  const p = { x: waypoint.x * 1, y: waypoint.y * 1 }; // position vector for waypoint
  const r = {
    x: Math.cos(waypoint.theta) /
    Math.sqrt(Math.cos(waypoint.theta) ** 2 + Math.sin(waypoint.theta) ** 2),
    y: Math.sin(waypoint.theta) /
    Math.sqrt(Math.cos(waypoint.theta) ** 2 + Math.sin(waypoint.theta) ** 2)
  }; // unit direction vector for waypoint
  const q = { x: platform.x * 1, y: platform.y * 1 }; // position vector for platfomr
  const s = {
    x: Math.cos(platform.theta) /
    Math.sqrt(Math.cos(platform.theta) ** 2 + Math.sin(platform.theta) ** 2),
    y: Math.sin(platform.theta) /
    Math.sqrt(Math.cos(platform.theta) ** 2 + Math.sin(platform.theta) ** 2)
  }; // unit direction vector for platform

  // CALCULATE PROJECTION
  const dist = Math.abs(Math.sqrt((p.x - q.x) ** 2 + (p.y - q.y) ** 2) *
  Math.cos(waypoint.theta - platform.theta));
  // CALCULATE INTERSECT
  const crossA = (q.x - p.x) * r.y - (q.y - p.y) * r.x; // (q-p) x r
  const crossB = r.x * s.y - r.y * s.x; // (rxs)

  const intersect = { x: 0, y: 0 };
  const target = { x: 0, y: 0 };
  let direction = 0;

  // vectors are collinear
  if (parseFloat(crossA).toFixed(3) === 0 && parseFloat(crossB).toFixed(3) === 0) {
    // return intersect at ideal intersect point to produce linear motion
    intersect.x = p.x - dist / 4 * r.x;
    intersect.y = p.y - dist / 4 * r.y;
    target.x = p.x - dist / 4 * r.x;
    target.y = p.y - dist / 4 * r.y;

    direction = Math.sign((target.x - intersect.x) / q.x);
  } else if (parseFloat(crossB).toFixed(3) === 0) { // vectors are parallel
    // no intersect
    intersect.x = NaN;
    intersect.y = NaN;
    target.x = NaN;
    target.y = NaN;

    direction = NaN;
  } else { // vectors intersect at one point
    // calculate intersect based on constant derived from cross products
    const u = crossA / crossB;
    intersect.x = q.x + u * s.x;
    intersect.y = q.y + u * s.y;
    target.x = p.x - dist / 4 * r.x;
    target.y = p.y - dist / 4 * r.y;

    direction = (target.x - intersect.x) / q.x;
  }

  return {
    intersect, target, direction, dist
  };
}

/**
 * Checks which side the platform is on in relation to a waypoint
 *
 * @param {Object} platformPos Position and orientation of platform
 * @param {Object} waypointPos Position and orientation of waypoint
 * @returns {Number} 1 = passed, 0 = on line, -1 = behind the line
 */
function checkSide(platformPos, waypointPos) {
  const wpNorm = { x: Math.cos(waypointPos.theta), y: Math.sin(waypointPos.theta) };
  const posDiff = { x: platformPos.x - waypointPos.x, y: platformPos.y - waypointPos.y };

  console.log('norm, diff: ', wpNorm, posDiff);

  const projection = posDiff.x * wpNorm.x + posDiff.y * wpNorm.y;

  console.log('projection', projection);

  const side = Math.sign(parseFloat(projection).toFixed(2));

  return side;
}

function getState(platformPos, waypointPos) {
  const waypointNorm = {
    x: waypointPos.x,
    y: waypointPos.y,
    theta: waypointPos.theta - Math.PI / 2
  };
  // determine side of waypoint that platform is on
  const side = checkSide(platformPos, waypointNorm); // -1 = left, 0 = on, +1 = right
  // determine whether platform is facing toward or away from the waypoint
  let facing = 0; // 1 = towards, -1 = away
  // platform is to the left of the waypoint
  console.log(mod(platformPos.theta - waypointPos.theta, 2 * Math.PI));
  const dTheta = mod(platformPos.theta - waypointPos.theta, 2 * Math.PI);

  if (side === 1 || side === -1) {
    if (dTheta >= 0 && dTheta <= Math.PI / 2 ||
      dTheta >= 3 * Math.PI / 2 && dTheta <= 2 * Math.PI) {
      facing = 1;
    } else {
      facing = -1;
    }
  // platform is in line with the waypoint
  } else if (parseFloat(platformPos.theta).toFixed(2) ===
  parseFloat(waypointPos.theta).toFixed(2)) {
    // if platform is facing in the same direction as the waypoint
    facing = 1;
  } else if (parseFloat(platformPos.theta).toFixed(2) ===
  parseFloat(mod(waypointPos.theta - Math.PI, 2 * Math.PI)).toFixed(2)) {
    facing = -1;
  } else {
    facing = 1;
  }
  // determine whether intersection between vectors overshoots
  // (1), undershoots (-1), or hits (0) the target point
  let mark = 0;
  const {
    intersect, target, direction, dist
  } = getIntersect(platformPos, waypointPos);
  console.log('Intersect: ', intersect, ' Distance: ', dist, ' Target: ', target);
  // if intersect exists
  if (!Number.isNaN(intersect.x)) {
    // if intersect and target are reasonably close
    if (Math.sqrt((intersect.x - target.x) ** 2 + (intersect.y - target.y) ** 2) < dist / 20) {
      mark = 0;
    } else if (direction >= 0) {
      mark = 1;
    } else {
      mark = -1;
    }
  } else {
    mark = -2;
  }

  return { side, facing, mark };
}

/**
 * Returns the new speed of the platform on its destination to a waypoint
 *
 * @param {Object} speed Speed of left and right motors
 * @param {Number} motorDistance Distance between motors (.6)
 * @param {any} platformPos (x,y,theta) of platform
 * @param {any} waypointPos (x,y,theta) of destination
 * @returns {Object} newSpeed Normarlized speed ratio
 */
function getRatio(
  speed, motorDistance, platformPos,
  waypointPos
) {
  const { side, facing, mark } = getState(platformPos, waypointPos);

  let ratio = [];

  const F = [1, 1];
  const BL = [0.25, 1];
  const BR = [1, 0.25];
  const TL = [-0.25, 1];
  const TR = [1, -0.25];
  const RO = [1, -1];
  // const LO = [-1, 1];

  switch (facing) {
    // if platform is facing away from waypoint
    case -1:
      switch (side) {
        // if platform is to the left of the waypoint
        case -1:
          switch (mark) {
            // if no intersect exists
            case -2:
              ratio = TL;
              break;
            // if intersect undershoots the target
            case -1:
              ratio = TL;
              break;
            // if intersect is within the target
            case 0:
              ratio = TL;
              break;
            // if intersect overshoots the target
            case 1:
              ratio = TL;
              break;
            default:
              ratio = F;
          }
          break;
        // if platform is in line with the waypoint
        case 0:
          switch (mark) {
            // if no intersect exists
            case -2:
              ratio = RO;
              break;
            // if intersect undershoots the target
            case -1:
              ratio = F;
              break;
            // if intersect is within the target
            case 0:
              ratio = RO;
              break;
            // if intersect overshoots the target
            case 1:
              ratio = F;
              break;
            default:
              ratio = F;
          }
          break;
        // if platform is to the right of the waypoint
        case 1:
          switch (mark) {
            // if no intersect exists
            case -2:
              ratio = TR;
              break;
            // if intersect undershoots the target
            case -1:
              ratio = TR;
              break;
            // if intersect is within the target
            case 0:
              ratio = TR;
              break;
            // if intersect overshoots the target
            case 1:
              ratio = TR;
              break;
            default:
              ratio = F;
          }
          break;
        default:
          ratio = F;
          break;
      }
      break;
    // if platform is facing towards waypoint
    case 1:
      switch (side) {
        // if platform is to the left of the waypoint
        case -1:
          switch (mark) {
            // if no intersect exists
            case -2:
              ratio = BR;
              break;
            // if intersect undershoots the target
            case -1:
              ratio = BL;
              break;
            // if intersect is within the target
            case 0:
              ratio = F;
              break;
            // if intersect overshoots the target
            case 1:
              ratio = BR;
              break;
            default:
              ratio = [1, 1];
          }
          break;
        // if platform is in line with the waypoint
        case 0:
          switch (mark) {
            // if no intersect exists
            case -2:
              ratio = F;
              break;
            // if intersect undershoots the target
            case -1:
              ratio = F;
              break;
            // if intersect is within the target
            case 0:
              ratio = F;
              break;
            // if intersect overshoots the target
            case 1:
              ratio = F;
              break;
            default:
              ratio = F;
          }
          break;
        // if platform is to the right of the waypoint
        case 1:
          switch (mark) {
            // if no intersect exists
            case -2:
              ratio = BL;
              break;
            // if intersect undershoots the target
            case -1:
              ratio = BR;
              break;
            // if intersect is within the target
            case 0:
              ratio = F;
              break;
            // if intersect overshoots the target
            case 1:
              ratio = BL;
              break;
            default:
              ratio = F;
          }
          break;
        default:
          ratio = F;
          break;
      }
      break;
    default:
      ratio = F;
      break;
  }
  console.log('SFM', side, facing, mark);
  return ratio;
}

export {
  checkSide, getRatio
};
