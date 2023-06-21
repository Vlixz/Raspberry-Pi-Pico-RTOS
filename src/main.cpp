int add()
{
  volatile int i = 0;

  while (1)
  {
    i++;
  }
}

int main()
{
  add();
}
