/*eslint-disable*/
import store from './store';
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
  const q = { x: platform.x * 1, y: platform.y * 1 }; // position vector for platform
  const s = {
    x: Math.cos(platform.theta) /
    Math.sqrt(Math.cos(platform.theta) ** 2 + Math.sin(platform.theta) ** 2),
    y: Math.sin(platform.theta) /
    Math.sqrt(Math.cos(platform.theta) ** 2 + Math.sin(platform.theta) ** 2)
  }; // unit direction vector for platform
  const r_orthagonal = {
    x: Math.cos(waypoint.theta + Math.PI / 2) /
    Math.sqrt(Math.cos(waypoint.theta + Math.PI / 2) ** 2 + Math.sin(waypoint.theta + Math.PI / 2) ** 2),
    y: Math.sin(waypoint.theta + Math.PI / 2) /
    Math.sqrt(Math.cos(waypoint.theta + Math.PI / 2) ** 2 + Math.sin(waypoint.theta + Math.PI / 2) ** 2)
  }; // unit direction vector orthagonal to waypoint direction vector

  // CALCULATE PROJECTION VECTORS
  const projLength = Math.sqrt((p.x - q.x) ** 2 + (p.y - q.y) ** 2) *
  Math.cos(waypoint.theta - platform.theta);
  const projection = { x: r.x * projLength, y: r.y * projLength };
  const rejection = { x: (p.x - q.x) - projection.x, y: (p.y - q.y) - projection.y };
  // const rejLength = Math.sqrt(rejection.x ** 2 + rejection.y ** 2);

  // CALCULATE INTERSECT
  const crossA = (q.x - p.x) * r.y - (q.y - p.y) * r.x; // (q-p) x r
  const crossB = r.x * s.y - r.y * s.x; // (rxs)

  // const crossC = (q.x - p.x) * r.y - (q.y - p.y) * r.x;
  // const crossD = r.x * r_orthagonal.y - r.y * r_orthagonal.x;

  let intersect = { x: 0, y: 0 };
  let target = { x: 0, y: 0 };
  let direction = 0;

  let dThetaWP = 0;
  let dThetaTarget = 0;
  let targetTheta = 0;

  // vectors are collinear
  if (parseFloat(crossA).toFixed(3) === 0 && parseFloat(crossB).toFixed(3) === 0) {
    // return intersect at ideal intersect point to produce linear motion
    intersect.x = p.x - projLength / 2 * r.x;
    intersect.y = p.y - projLength / 2 * r.y;
    target.x = p.x - projLength / 2 * r.x;
    target.y = p.y - projLength / 2 * r.y;

    direction = Math.sign((target.x - intersect.x) / q.x);

    dThetaWP = 0;
    dThetaTarget = 0;
  } else if (parseFloat(crossB).toFixed(3) === 0) { // vectors are parallel
    // no intersect
    intersect.x = NaN;
    intersect.y = NaN;
    target.x = NaN;
    target.y = NaN;

    direction = NaN;

    dThetaWP = 0;
    dThetaTarget = 0;
  } else { // vectors intersect at one point
    // calculate intersect based on constant derived from cross products
    const u = crossA / crossB;
    intersect.x = q.x + u * s.x;
    intersect.y = q.y + u * s.y;
    target.x = p.x - projLength / 2 * r.x;
    target.y = p.y - projLength / 2 * r.y;

    direction = (target.x - intersect.x) / q.x;

    targetTheta = mod(Math.atan2(target.y - platform.y, target.x - platform.x), 2 * Math.PI);

    dThetaWP = waypoint.theta - platform.theta;
    dThetaTarget = targetTheta - platform.theta;
  }

  return {
    intersect, target, direction, projection, rejection, dThetaWP, dThetaTarget
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

  // console.log('norm, diff: ', wpNorm, posDiff);

  const projection = posDiff.x * wpNorm.x + posDiff.y * wpNorm.y;

  // console.log('projection', projection);

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
  let quadrant = -1; /* 0 = Facing with waypoint towards line,
                       1 = Facing with waypoint away from line,
                       2 = Facing against waypoint towards line,
                       3 = Facing against waypoint away form line */

  // platform is to the left of the waypoint
  console.log(mod(platformPos.theta - waypointPos.theta, 2 * Math.PI));
  const dTheta = mod(platformPos.theta - waypointPos.theta, 2 * Math.PI);

  // platform is to the left of the waypoint
  if (side === -1) {
    // Quadrant 0
    if (2 * Math.PI > dTheta && dTheta >= 3 * Math.PI / 2) {
      quadrant = 0;
    }
    // Quadrant 1
    else if (Math.PI / 2 > dTheta && dTheta >= 0) {
      quadrant = 1;
    }
    // Quadrant 2
    else if (3 * Math.PI / 2 > dTheta && dTheta >= Math.PI) {
      quadrant = 2;
    }
    // Quadrant 3
    else {
      quadrant = 3;
    }

  // platform is to the right of the waypoint
  } else if (side === 1) {
    // Quadrant 0
    if (Math.PI / 2 >= dTheta && dTheta > 0) {
      quadrant = 0;
    }
    // Quadrant 1
    else if ((dTheta === 0 || 2 * Math.PI >= dTheta) && dTheta > 3 * Math.PI / 2) {
      quadrant = 1;
    }
    // Quadrant 2
    else if (Math.PI >= dTheta && dTheta > Math.PI / 2) {
      quadrant = 2;
    }
    // Quadrant 3
    else {
      quadrant = 3;
    }

  // platform is in line with the waypoint
  } else {
    // platform is facing with waypoint
    if (Math.PI / 2 > dTheta && dTheta >= 0 || 2 * Math.PI > dTheta && dTheta > 3 * Math.PI / 2) {
      quadrant = 0;
    }
    // platform is facing against waypoint
    else {
      quadrant = 2;
    }
  }
  // determine whether intersection between vectors overshoots
  // (1), undershoots (-1), or hits (0) the target point
  let mark = 0;
  const {
    intersect, target, direction, projection, rejection, dThetaWP, dThetaTarget
  } = getIntersect(platformPos, waypointPos);

  const projLength = Math.sqrt(projection.x ** 2 + projection.y ** 2);
  console.log(rejection);
  // const rejLength = Math.sqrt(rejection.x ** 2 + rejection.y ** 2);
  console.log('Intersect: ', intersect, ' Target: ', target);
  // store.commit('CHANGE_CUE', {waypoints: [{ x: target.x, y: target.y }]});
  // store.commit('CHANGE_TARGET', { x: intersect.x, y: intersect.y });

  // if intersect exists
  if (!Number.isNaN(intersect.x)) {
    // if intersect and target are reasonably close
    if (Math.sqrt((intersect.x - target.x) ** 2 +
    (intersect.y - target.y) ** 2) < projLength / 20) {
      mark = 0;
    } else if (direction >= 0) {
      mark = 1;
    } else {
      mark = -1;
    }
  } else {
    mark = -2;
  }

  return { side, quadrant, mark, dThetaWP, dThetaTarget };
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
  const {
    side, quadrant, mark, dThetaWP, dThetaTarget
  } = getState(platformPos, waypointPos);

  let ratio = [];

  const diffSpeed = Math.cos(dThetaTarget) ** 3;

  const E = [1, 1]; // Ratio for cases that should not occur.
  const F = [1, 1];
  const BL = [diffSpeed, 1];
  const BR = [1, diffSpeed];
  const TL = [diffSpeed, 1];
  const TR = [1, diffSpeed];
  // const RO = [1, -1];
  // const LO = [-1, 1];

  switch (quadrant) {
    // platform is facing with the waypoint and towards the line
    case 0:
      switch (side) {
        // if platform is to the left of the waypoint
        case -1:
          switch (mark) {
            // if no intersect exists
            case -2:
              ratio = E;
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
              ratio = E;
          }
          break;
        // if platform is in line with the waypoint
        case 0:
          ratio = F;
          break;
        // if platform is to the right of the waypoint
        case 1:
          switch (mark) {
            // if no intersect exists
            case -2:
              ratio = E;
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
              ratio = E;
          }
          break;
        default:
          ratio = E;
          break;
      }
      break;
    // platform is facing with the waypoint and away from the line
    case 1:
      switch (side) {
        // if platform is to the left of the waypoint
        case -1:
          ratio = TR;
          break;
        // if platform is in line with the waypoint
        case 0:
          ratio = E;
          break;
        // if platform is to the right of the waypoint
        case 1:
          ratio = TL;
          break;
        default:
          ratio = E;
          break;
      }
      break;
    // platform is facing against the waypoint and towards the line
    case 2:
      switch (side) {
        // if platform is to the left of the waypoint
        case -1:
          ratio = TL;
          break;
        // if platform is in line with the waypoint
        case 0:
          ratio = F;
          break;
        // if platform is to the right of the waypoint
        case 1:
          ratio = TR;
          break;
        default:
          ratio = E;
          break;
      }
      break;
    // platform is facing against the waypoint and away from the line
    case 3:
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
              ratio = TR;
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
              ratio = E;
          }
          break;
        // if platform is in line with the waypoint
        case 0:
          ratio = E;
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
              ratio = TL;
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
              ratio = E;
          }
          break;
        default:
          ratio = E;
          break;
      }
      break;
    default:
      ratio = E;
      break;
  }
  console.log('SQM', side, quadrant, mark);
  console.log('ratio', ratio);
  // console.log('ratio', ratio);
  return ratio;
}

export {
  checkSide, getRatio
};
