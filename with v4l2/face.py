import cv2
import dlib


hog_face_detector = dlib.get_frontal_face_detector()


while True:
    frame = cv2.imread('out0.jpeg')
    gray = cv2.imread('out0.jpeg',0)

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
