import cv2
from ultralytics import YOLO

model = YOLO('model/yolov8n.pt', "v8")
model.info()
frame_width = 1280
frame_height = 720

cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print('cannot open video stream')
    exit()

while True:
    ret, frame = cap.read()

    if not ret:
        print('No video frame available')
        break

    frame = cv2.resize(frame, (frame_width, frame_height))

    detect_params = model.predict(source=[frame], conf=0.8, save=False)

    DP = detect_params[0].numpy()

    if len(DP) != 0:
        for i in range(len(detect_params[0])):
            boxes = detect_params[0].boxes
            box = boxes[i]
            clsID = box.cls.numpy()[0]
            conf = box.conf.numpy()[0]
            bb = box.xyxy.numpy()[0]
            c = box.cls

            class_name = model.names[int(c)]

        if 'bird' in class_name.lower():
            cv2.rectangle(frame, (int(bb[0]), int(bb[1]), int(bb[2]), int(bb[3])), (0, 255,0), 3)

            font = cv2.FONT_HERSHEY_SIMPLEX
            cv2.putText(frame, class_name+" "+str(round(conf, 3))+"%", (int(bb[0]), int(bb[1]) - 10), font, 1,(255,255,255), 2)


    cv2.imshow("Object detection", frame)

    if cv2.waitKey(1) == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()