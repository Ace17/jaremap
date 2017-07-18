
class Filter
{
  void processData()
  {
    Generator gen = new Generator();

    gen.doGenerate();
  }

  int setFilterParams(FilterParams params)
  {
    return params.a;
  }

  void FilterParams()
  {
  }

  int m_integerField;
}

