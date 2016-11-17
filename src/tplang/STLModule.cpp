#include "STLModule.h"

using namespace cb;
using namespace tplang;
using namespace CAMotics;

void STLModule::define(js::ObjectTemplate &exports) {
  exports.set("makeContour(level,fileName)", this, &STLModule::makeContourCB);
}

bool isBetween(float level,float z1,float z2) {
  if (((z1 >= level) && (level >= z2)) || ((z2 >= level) && (level >= z1))) return true;
  return false;
}

float distanceBetweenPoints(cb::Vector2F &p1,cb::Vector2F &p2) {
  float distance;
  distance = std::sqrt((p1[0] - p2[0]) * (p1[0] - p2[0]) +
                       (p1[1] - p2[1]) * (p1[1] - p2[1]));
  if (distance < 0.001) return(0.0);
  return(distance);
};

bool getSegment(float level,
                cb::Vector3F &normal,
                cb::Vector3F &v1,cb::Vector3F &v2,cb::Vector3F &v3,
                cb::Vector2F &p1,cb::Vector2F &p2) {
  float ratio;
  bool p1Taken, p2Taken;
  //skip facets that lay on the "level" plane
  if (v1[2] == level && v2[2] == level && v3[2] == level) return(false);
  //skip faces with and edge on the "level" plane and hang down
  if (((v1[2] == level) && (v2[2] == level) && (v3[2] < level)) ||
      ((v1[2] == level) && (v3[2] == level) && (v2[2] < level)) ||
      ((v2[2] == level) && (v3[2] == level) && (v1[2] < level))) return(false);
  p1Taken = false;
  p2Taken = false;
  if (isBetween(level,v1[2],v2[2])) {
    if(v1[2] == v2[2]) { //edge is on the plane
      if(v3[1] > level) return(false);
      p1[0] = v1[0];
      p1[1] = v1[1];
      p2[0] = v2[0];
      p2[1] = v2[1];
      return(true);
    };
    ratio = (level - v2[2])/(v1[2] - v2[2]);
    p1[0] = v2[0] + ratio * (v1[0] - v2[0]);
    p1[1] = v2[1] + ratio * (v1[1] - v2[1]);
    p1Taken = true;
  }
  if (isBetween(level,v2[2],v3[2])) {
    if(v2[2] == v3[2]) { //edge is on the plane
      if(v1[1] > level) return(false);
      p1[0] = v2[0];
      p1[1] = v2[1];
      p2[0] = v3[0];
      p2[1] = v3[1];
      return(true);
    };
    ratio = (level - v3[2])/(v2[2] - v3[2]);
    if(!p1Taken) {
      p1[0] = v3[0] + ratio * (v2[0] - v3[0]);
      p1[1] = v3[1] + ratio * (v2[1] - v3[1]);
      p1Taken = true;
    } else {
      p2[0] = v3[0] + ratio * (v2[0] - v3[0]);
      p2[1] = v3[1] + ratio * (v2[1] - v3[1]);
      p2Taken = true;
    }
  }
  if(!p1Taken) return(false);
  if (isBetween(level,v3[2],v1[2])) {
    ratio = (level - v1[2])/(v3[2] - v1[2]);
    if(!p2Taken) {
      p2[0] = v1[0] + ratio * (v3[0] - v1[0]);
      p2[1] = v1[1] + ratio * (v3[1] - v1[1]);
      p2Taken = true;
    };
  };
  if (!p2Taken) {
    return(false);
  };
  if(distanceBetweenPoints(p1,p2) == 0.0) return(false); //don't return single point segs
  return(true);
};

void swap(cb::Vector2F &p1,cb::Vector2F &p2) {
  float temp[2];
  temp[0] = p1[0], temp[1] = p1[1];
  p1[0] = p2[0], p1[1] = p2[1];
  p2[0] = temp[0], p2[1] = temp[1];
};

bool IsEqual(float x, float y) {
  if (y == 0.0) {
    if (fabs(x) < 0.00001) return true;
    else return false;
  };
  if (fabs((x-y)/y) < .00001) return true;
  return false;
};

struct SEG {
  cb::Vector2F p1;
  cb::Vector2F p2;
};
  

/* makeContourCB creates a list of all contours at the level given by "level".
 * 
 * It starts by loading each facet and looking at each edge of the triangle
 * to determine whether the facet crosses "level".
 * 
 * If so, it determines the line segment that is formed by the intersection
 * of z = level and the facet.
 * 
 * It uses normal to determine the direction of the line segment (i.e. which
 * is x1,y1 and which is x2,y2).  When traveling from x1,y1 to x2,y2, the normal
 * vector must always lean to the right to ensure that the polygon progresses
 * counterclockwise direction.
 *
 * After all line segments have been recorder, they are sorted to form closed
 * polygons.args.
 *
 * Each time a polygon closes, it is recorded into the return string
 * and a new polygon is started.
 *
 * If any polygons fail to close (within tolerance), an error is generated.
 *
 * The returned string is of the form [[{X: x1, Y: y1},...{X: xn, Y: yn}],
 * ...[{X: x1, Y: y1},...{X: xn, Y: yn}]]
 */  
js::Value STLModule::makeContourCB(const js::Arguments &args) {
  SEG seg;
  std::vector<SEG> segments;
  std::vector<SEG>::iterator itr;
  float level = args.getNumber("level");
  cb::Vector3F v1, v2, v3, normal;
  cb::Vector2F p1, p2, first, last;
  std::ostringstream returnString;
  std::string name;
  std::string hash;
  try {
    InputSource s = InputSource((const std::string &) args.getString("fileName"));
    reader = new STLReader(s);
    reader->readHeader(name,hash);
  } catch (...) {
    delete reader;
    reader = NULL;
    return("Error, can't open file");
  }  
  returnString << "";
  while(reader->hasMore()) {
    reader->readFacet(v1,v2,v3,normal);
    if(((v1[2] > level) && (v2[2] > level) && (v3[2] > level)) ||
       ((v1[2] < level) && (v2[2] < level) && (v3[2] < level))) { continue;};

    if(getSegment(level,normal,v1,v2,v3,p1,p2)) {
      if (IsEqual(p1[0],p2[0])) {
        if (p1[1] < p2[1]) {
          if (normal[0] < 0) swap(p1,p2);
        } else {
          if (normal[0] > 0) swap(p1,p2);
        }
      } else if (IsEqual(p1[1],p2[1])) {
        if (p1[0] < p2[0]) {
          if (normal[1] > 0) swap(p1,p2);
        } else {
          if (normal[1] < 0) swap(p1,p2);        
        }       
      } else if (p1[0] < p2[0] && p1[1] < p2[1]) {
        if(normal[0] < 0) swap(p1,p2);
      } else if (p1[0] > p2[0] && p1[1] > p2[1]) {
        if (normal[0] > 0) swap(p1,p2);
      } else if (p1[0] < p2[0] && p1[1] > p2[1]) {
        if (normal[0] > 0) swap(p1,p2);
      } else {
        if (normal[0] < 0) swap(p1,p2);
      }
      seg.p1[0] = p1[0],seg.p1[1] = p1[1],seg.p2[0] = p2[0],seg.p2[1] = p2[1];
      if (segments.capacity() < segments.size() + 10) segments.reserve(segments.capacity() + 100);
      segments.push_back(seg);
    };
  };
  delete reader;
  reader = NULL;
  returnString << "[";
  if (segments.size() == 0) {
    returnString << "]";
    return (returnString.str());
  };
  returnString << "[";
  first = segments.begin()->p1;
  last = segments.begin()->p2;
  returnString << "{\"X\": " << first[0] << ",\"Y\": " << first[1] << "}, ";
  returnString << "{\"X\": " << last[0] << ",\"Y\": " << last[1] << "}";
  segments.erase(segments.begin());
  float d1, d2;
  std::vector<SEG>::iterator tempItr, closestItr;
  while(segments.size() > 0) {
    if (returnString.str().size() > (returnString.str().capacity() - 100))
      returnString.str().reserve(returnString.str().size() + 100);
    tempItr = segments.begin();
    d1 = distanceBetweenPoints(last,tempItr->p1);
    closestItr = tempItr;
    while(!(tempItr == segments.end())) {
      tempItr++;
      d2 = distanceBetweenPoints(last, tempItr->p1);
      if (d2 < d1) {
        closestItr = tempItr;
        d1 = d2;
      };
    };
    last[0] = closestItr->p2[0], last[1] = closestItr->p2[1];
    returnString << ", {\"X\": " << last[0] << ",\"Y\": " << last[1] << "}";
    segments.erase(closestItr);
    if(distanceBetweenPoints(last,first) == 0) {
      returnString << "]";
      if (segments.size() > 0) {
        first = segments.begin()->p1;
        last = segments.begin()->p2;
        returnString << ", [";
        returnString << "{\"X\": " << first[0] << ",\"Y\": " << first[1] << "}, ";
        returnString << "{\"X\": " << last[0] << ",\"Y\": " << last[1] << "}";
        segments.erase(segments.begin());
      };
    };

  };
  returnString << "]";
  return(returnString.str());
}
