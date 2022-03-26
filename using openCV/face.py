import cv2
import dlib

cap = cv2.VideoCapture(0)

hog_face_detector = dlib.get_frontal_face_detector()

dlib_facelandmark = dlib.shape_predictor("shape_predictor_68_face_landmarks.dat")

while True:
    _, frame = cap.read()
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    faces = hog_face_detector(gray)
    
    for face in faces:
    	x,y = face.left(), face.top()
    	x1,y1 = face.right(), face.bottom()
    	cv2.rectangle(frame, (x,y), (x1,y1), (0, 255, 100), 2)  

    cv2.imshow("Face", frame)

    key = cv2.waitKey(1)
    if key == 27:
        break
        
        
cap.release()
cv2.destroyAllWindows()
