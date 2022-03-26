import cv2
import dlib

hog_face_detector = dlib.get_frontal_face_detector()

dlib_facelandmark = dlib.shape_predictor("shape_predictor_68_face_landmarks.dat")

while True:
    frame = cv2.imread('out0.jpeg')
    
    gray = cv2.imread('out0.jpeg',0)
    
    faces = hog_face_detector(gray)
    
    for face in faces:

        face_landmarks = dlib_facelandmark(gray, face)

        for n in range(0, 16):
            x = face_landmarks.part(n).x
            y = face_landmarks.part(n).y
            cv2.circle(frame, (x, y), 1, (0 ,255, 100), 1)


    cv2.imshow("Face Landmarks on image read", frame)

    key = cv2.waitKey(1)
    if key == 27:
        break
cap.release()
cv2.destroyAllWindows()
