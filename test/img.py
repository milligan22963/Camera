from PIL import Image

def main():
  try:
    image = Image.open("/home/daniel/Pictures/buddy.jpg");
  except IOError:
    print("Error")
    pass

if __name__ == "__main__":
  print("Starting main")
  main()
