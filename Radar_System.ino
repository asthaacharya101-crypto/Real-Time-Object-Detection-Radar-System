
import processing.serial.*;
import java.util.ArrayList;

Serial arduino;

int scanAngle = 90;
int distanceCm = -1;

final int MAX_DISTANCE = 200;
final int TRAIL_LIFETIME = 2500;

ArrayList<Detection> detections =
  new ArrayList<Detection>();

class Detection {
  int angle;
  int distance;
  int createdAt;

  Detection(int a, int d) {
    angle = a;
    distance = d;
    createdAt = millis();
  }
}

void setup() {
  size(900, 650);
  smooth();

  println(Serial.list());

  // Change COM12 if your Arduino uses another port.
  arduino = new Serial(this, "COM12", 9600);
  arduino.bufferUntil('\n');
}

void draw() {
  background(0);

  drawRadarGrid();
  drawSweepLine();
  drawDetections();
  drawInformation();
}

void serialEvent(Serial port) {
  String line = port.readStringUntil('\n');

  if (line == null) return;

  line = trim(line);
  String[] values = split(line, ',');

  if (values.length < 2) return;

  try {
    int angle = int(trim(values[0]));
    int distance = int(trim(values[1]));

    if (angle < 0 || angle > 180) return;

    scanAngle = angle;
    distanceCm = distance;

    if (distance > 0 &&
        distance <= MAX_DISTANCE) {

      boolean addPoint = true;

      if (detections.size() > 0) {
        Detection last =
          detections.get(detections.size() - 1);

        if (abs(last.angle - angle) < 4 &&
            abs(last.distance - distance) < 5) {
          addPoint = false;
        }
      }

      if (addPoint) {
        detections.add(new Detection(angle, distance));
      }
    }
  }
  catch (Exception e) {
    // Ignore invalid lines and header text.
  }
}

float centerX() {
  return width / 2.0;
}

float centerY() {
  return height - 55;
}

float radarRadius() {
  return min(width * 0.46, height * 0.76);
}

void drawRadarGrid() {
  float cx = centerX();
  float cy = centerY();
  float rMax = radarRadius();

  stroke(0, 180, 65);
  strokeWeight(1);
  noFill();

  // Distance rings.
  for (int i = 1; i <= 4; i++) {
    float r = rMax * i / 4.0;
    arc(cx, cy, 2*r, 2*r, PI, TWO_PI);
  }

  // Angle lines.
  for (int a = 0; a <= 180; a += 30) {
    float rad = radians(180 - a);

    float x = cx + rMax * cos(rad);
    float y = cy - rMax * sin(rad);

    line(cx, cy, x, y);
  }

  fill(0, 255, 90);
  noStroke();
  textAlign(CENTER);
  textSize(12);

  for (int d = 50; d <= MAX_DISTANCE; d += 50) {
    float r = rMax * d / MAX_DISTANCE;
    text(d + " cm", cx + 35, cy - r + 15);
  }

  text("180", cx - rMax, cy + 20);
  text("90", cx, cy - 8);
  text("0", cx + rMax, cy + 20);
}

void drawSweepLine() {
  float cx = centerX();
  float cy = centerY();
  float rMax = radarRadius();

  float rad = radians(180 - scanAngle);

  float x = cx + rMax * cos(rad);
  float y = cy - rMax * sin(rad);

  stroke(0, 255, 80);
  strokeWeight(3);
  line(cx, cy, x, y);
}

void drawDetections() {
  float cx = centerX();
  float cy = centerY();
  float rMax = radarRadius();

  for (int i = detections.size() - 1; i >= 0; i--) {
    Detection det = detections.get(i);

    int age = millis() - det.createdAt;

    if (age > TRAIL_LIFETIME) {
      detections.remove(i);
      continue;
    }

    float fade = 1.0 -
      (float)age / TRAIL_LIFETIME;

    int alpha = (int)(255 * fade);

    float r = rMax * det.distance / MAX_DISTANCE;
    float rad = radians(180 - det.angle);

    float x = cx + r * cos(rad);
    float y = cy - r * sin(rad);

    noStroke();
    fill(255, 35, 35, alpha);
    ellipse(x, y, 12, 12);

    fill(255, 120, 120, alpha);
    textAlign(LEFT);
    textSize(11);
    text(det.distance + " cm", x + 8, y - 5);
  }
}

void drawInformation() {
  fill(0, 255, 90);
  textAlign(LEFT);

  textSize(21);
  text("ULTRASONIC RADAR", 20, 30);

  textSize(14);
  text("Angle: " + scanAngle + " degrees", 20, 57);

  if (distanceCm > 0) {
    text("Distance: " + distanceCm + " cm", 20, 80);
  } else {
    text("Distance: No echo", 20, 80);
  }

  text("Display range: 0-" +
    MAX_DISTANCE + " cm", 20, 103);

  text("Recent detections: " +
    detections.size(), 20, 126);

  textSize(11);
  text("Red dots fade after 2.5 seconds",
    20, height - 15);
}
