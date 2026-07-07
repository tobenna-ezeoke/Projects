from picamera2 import Picamera2
import cv2
import numpy as np
import time

# Initialize PiCamera2
picam2 = Picamera2()
picam2.preview_configuration.main.size = (640, 480)
picam2.preview_configuration.main.format = "RGB888"
picam2.preview_configuration.align()
picam2.configure("preview")
picam2.start()

time.sleep(1)  # Camera warm-up

def classify_object(roi):
    hsv = cv2.cvtColor(roi, cv2.COLOR_RGB2HSV)

    lower_paper = np.array([0, 0, 200])     # White/Brown range
    upper_paper = np.array([180, 50, 255])

    lower_green = np.array([35, 40, 40])    # Green for compost
    upper_green = np.array([85, 255, 255])

    lower_metal = np.array([0, 0, 80])      # Generic plastic/metal
    upper_metal = np.array([180, 50, 200])

    mask_paper = cv2.inRange(hsv, lower_paper, upper_paper)
    mask_compost = cv2.inRange(hsv, lower_green, upper_green)
    mask_metal = cv2.inRange(hsv, lower_metal, upper_metal)

    paper_area = cv2.countNonZero(mask_paper)
    compost_area = cv2.countNonZero(mask_compost)
    metal_area = cv2.countNonZero(mask_metal)

    if paper_area > 500:
        return "Paper"
    elif compost_area > 500:
        return "Compost"
    elif metal_area > 500:
        return "Metal/Plastic"
    else:
        return "Landfill"

first_frame = None

print("Starting motion detection + classification test...")

while True:
    frame = picam2.capture_array()
    gray = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
    gray = cv2.GaussianBlur(gray, (21, 21), 0)

    if first_frame is None:
        first_frame = gray
        continue

    frame_delta = cv2.absdiff(first_frame, gray)
    thresh = cv2.threshold(frame_delta, 25, 255, cv2.THRESH_BINARY)[1]
    thresh = cv2.dilate(thresh, None, iterations=2)
    contours, _ = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    motion_detected = False
    for contour in contours:
        if cv2.contourArea(contour) < 500:
            continue
        motion_detected = True
        (x, y, w, h) = cv2.boundingRect(contour)
        roi = frame[y:y+h, x:x+w]
        category = classify_object(roi)

        print(f"Motion Detected! Classified as: {category}")

        # Draw debug box
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        cv2.putText(frame, category, (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)

    if motion_detected is False:
        print("No motion detected.")

    cv2.imshow("Test Feed", cv2.cvtColor(frame, cv2.COLOR_RGB2BGR))

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

picam2.stop()
cv2.destroyAllWindows()
